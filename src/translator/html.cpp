#include "html.h"

#include "response.h"
#include "xh_scanner.h"

namespace {
using marian::string_view;
using marian::bergamot::AnnotatedText;
using marian::bergamot::ByteRange;
using marian::bergamot::HTML;
using marian::bergamot::Response;

void encodeEntities(string_view const &input, std::string &output) {
  output.clear();
  output.reserve(input.size());

  for (auto it = input.begin(); it != input.end(); ++it) {
    switch (*it) {
      case '&':
        output.append("&amp;");
        break;
      case '<':
        output.append("&lt;");
        break;
      case '>':
        output.append("&gt;");
        break;
      // case ???:
      //   output.append("&nbsp;");
      //   break;
      // case '"':
      //   output.append("&quot;");
      //   break;
      // case '\'':
      //   output.append("&apos;");
      //   break;
      default:
        output.push_back(*it);
        break;
    }
  }
}

size_t countPrefixWhitespaces(string_view const &input) {
  size_t size = 0;
  while (size < input.size() && input[size] == ' ') ++size;
  return size;
}

std::ostream &operator<<(std::ostream &out, HTML::Tag const *tag) {
  if (tag == nullptr) return out << "[nullptr]";
  out << '<' << tag->name << tag->attributes;
  if (tag->empty) out << '/';
  return out << '>';
}

std::ostream &operator<<(std::ostream &out, HTML::Taint const &tags) {
  for (auto it = tags.begin(); it != tags.end(); ++it) {
    if (it != tags.begin()) out << ' ';
    out << *it;
  }
  return out;
}

// Very simple replacement for std::format introduced in C++20
std::string format(std::string const &formatTemplate) { return formatTemplate; }

template <typename Arg>
std::string format(std::string const &formatTemplate, Arg arg) {
  std::ostringstream os;
  auto index = formatTemplate.find("{}");
  assert(index != std::string::npos);
  os << formatTemplate.substr(0, index) << arg << formatTemplate.substr(index + 2);
  return os.str();
}

template <typename Arg, typename... Args>
std::string format(std::string const &formatTemplate, Arg arg, Args... args) {
  std::ostringstream os;
  auto index = formatTemplate.find("{}");
  assert(index != std::string::npos);
  os << formatTemplate.substr(0, index) << arg << format(formatTemplate.substr(index + 2), std::forward<Args>(args)...);
  return os.str();
}

bool isBlockElement(std::string_view const &name) {
  // List of elements that we expect might occur inside words, and that should
  // not introduce spacings around them. Not strictly inline elements, nor flow
  // elements. See also https://developer.mozilla.org/en-US/docs/Web/Guide/HTML/Content_categories
  static std::unordered_set<std::string> inlineishElements{
      "abbr",  "a",    "b",      "em",  "i",   "kbd",  "mark", "math", "output", "q",   "ruby",
      "small", "span", "strong", "sub", "sup", "time", "u",    "var",  "wbr",    "ins", "del"};

  return inlineishElements.find(std::string(name)) == inlineishElements.end();
}

bool isVoidTag(std::string_view const &name) {
  // List of elements for which we do not expect a closing tag, or self-closing
  // elements in XHTML. See also https://developer.mozilla.org/en-US/docs/Glossary/Empty_element
  // More relevant source of this list:
  // https://searchfox.org/mozilla-central/rev/7d17fd1fe9f0005a2fb19e5d53da4741b06a98ba/dom/base/FragmentOrElement.cpp#1791
  static std::unordered_set<std::string> voidElements{"area",  "base",  "basefont", "bgsound", "br",    "col",
                                                      "embed", "frame", "hr",       "img",     "input", "keygen",
                                                      "link",  "meta",  "param",    "source",  "track", "wbr"};

  return voidElements.find(std::string(name)) != voidElements.end();
}

void diffTags(HTML::Taint const &prev, HTML::Taint const &curr, HTML::Taint &opening, HTML::Taint &closing) {
  opening.clear();
  closing.clear();

  size_t i = 0;

  // Find first difference
  for (; i < prev.size(); ++i)
    if (i >= curr.size() || prev[i] != curr[i]) break;

  std::copy_if(prev.begin() + i, prev.end(), std::back_inserter(closing), [&](HTML::Tag *tag) { return !tag->empty; });

  opening.insert(opening.end(), curr.begin() + i, curr.end());
}

bool intersects(ByteRange const &range, HTML::Span const &span) {
  return range.begin <= span.end && range.end >= span.begin;
};

void filterEmpty(HTML::Taint &stack) {
  auto src = stack.begin();
  auto dst = stack.begin();

  for (auto src = stack.begin(); src != stack.end(); ++src)
    if (!(*src)->empty) *(dst++) = *src;

  stack.resize(dst - stack.begin());
}

bool containsTag(HTML::Taint const &stack, HTML::Tag const *tag) {
  return std::find(stack.rbegin(), stack.rend(), tag) != stack.rend();
}

template <typename Fun>
AnnotatedText apply(AnnotatedText const &in, Fun fun) {
  AnnotatedText out;

  for (size_t sentenceIdx = 0; sentenceIdx < in.numSentences(); ++sentenceIdx) {
    std::string sentence;
    std::vector<ByteRange> tokens;

    std::string prefix = fun(in.annotation.gap(sentenceIdx), in.gap(sentenceIdx), false);

    for (size_t wordIdx = 0; wordIdx < in.numWords(sentenceIdx); ++wordIdx) {
      std::string token = fun(in.wordAsByteRange(sentenceIdx, wordIdx), in.word(sentenceIdx, wordIdx), false);
      tokens.push_back(ByteRange{sentence.size(), sentence.size() + token.size()});
      sentence += token;
    }

    // Convert our ByteRanges to string_views since that's what appendSentence
    // expects
    // TODO: extend AnnotatedText::appendSentence to accept str + ByteRanges
    // directly
    std::vector<string_view> token_views(tokens.size());
    std::transform(tokens.begin(), tokens.end(), token_views.begin(),
                   [&](ByteRange const &range) { return string_view(sentence.data() + range.begin, range.size()); });

    out.appendSentence(prefix, token_views.begin(), token_views.end());
  }

  out.appendEndingWhitespace(fun(in.annotation.gap(in.numSentences()), in.gap(in.numSentences()), true));

  return out;
}

bool isContinuation(string_view str) { return !str.empty() && str.compare(0, 1, " ", 1) != 0; }

bool hasAlignments(Response const &response) {
  // Test for each sentence individually as a sentence may be empty (or there)
  // might be no sentences, so just testing for alignments.empty() would not be
  // sufficient.
  for (size_t sentenceIdx = 0; sentenceIdx < response.target.numSentences(); ++sentenceIdx) {
    // If response.alignments is just empty, this might catch it.
    if (response.alignments.size() <= sentenceIdx ||
        response.alignments[sentenceIdx].size() != response.target.numWords(sentenceIdx))
      return false;

    // If response.alignments is "empty" because the model did not provide alignments,
    // it still has entries for each target word. But all these entries are empty.
    for (size_t wordIdx = 0; wordIdx < response.target.numWords(sentenceIdx); ++wordIdx)
      if (response.alignments[sentenceIdx][wordIdx].size() != response.source.numWords(sentenceIdx)) return false;
  }
  return true;
}

void hardAlignments(Response const &response, std::vector<std::vector<size_t>> &alignments) {
  // For each sentence...
  for (size_t sentenceIdx = 0; sentenceIdx < response.target.numSentences(); ++sentenceIdx) {
    alignments.emplace_back();

    // Hard-align: find for each target token the most prevalent source token
    // Note: only search from 0 to N-1 because token N is end-of-sentence token
    // that can only align with the end-of-sentence token of the target
    for (size_t t = 0; t + 1 < response.target.numWords(sentenceIdx); ++t) {
      size_t s_max = 0;
      for (size_t s = 1; s + 1 < response.source.numWords(sentenceIdx); ++s) {
        if (response.alignments[sentenceIdx][t][s] > response.alignments[sentenceIdx][t][s_max]) {
          s_max = s;
        }
      }

      alignments.back().push_back(s_max);
    }

    // Next, we try to smooth out these selected alignments with a few heuristics
    for (size_t t = 1; t + 1 < response.target.numWords(sentenceIdx); ++t) {
      // If this token is a continuation of a previous token, pick the tags from the most
      // prevalent token for the whole word.
      if (isContinuation(response.target.word(sentenceIdx, t))) {
        // Note: only looking at the previous token since that will already
        // have this treatment applied to it.
        size_t currSentenceIdx = alignments.back()[t];
        size_t prevSentenceIdx = alignments.back()[t - 1];
        float currScore = response.alignments[sentenceIdx][t][currSentenceIdx];
        float prevScore = response.alignments[sentenceIdx][t - 1][prevSentenceIdx];

        if (currScore > prevScore) {
          // Apply this to all previous tokens in the word
          for (size_t i = t;; --i) {
            alignments.back()[i] = currSentenceIdx;

            // Stop if this was the first token or the beginning of the word
            if (i == 0 || !isContinuation(response.target.word(sentenceIdx, i))) break;
          }
        } else {
          alignments.back()[t] = prevSentenceIdx;
        }
      }
    }

    // Always align target end with source end
    alignments.back().push_back(response.source.numWords(sentenceIdx) - 1);
  }
}

void copyTaint(Response const &response, std::vector<std::vector<size_t>> const &alignments,
               std::vector<HTML::Taint> const &sourceTokenTags, std::vector<HTML::Taint> &targetTokenTags) {
  size_t offset = 0;

  // Fill targetTokenTags based on the alignments we just made up.
  // NOTE: this should match the exact order of Apply()
  for (size_t sentenceIdx = 0; sentenceIdx < response.target.numSentences(); ++sentenceIdx) {
    targetTokenTags.push_back(sourceTokenTags[offset]);  // token_tag for sentence ending gap
    for (size_t t = 0; t < response.target.numWords(sentenceIdx); ++t) {
      size_t s = alignments[sentenceIdx][t];
      assert(s < response.source.numWords(sentenceIdx));
      targetTokenTags.push_back(sourceTokenTags[offset + 1 + s]);  // +1 for prefix gap
    }

    offset += response.source.numWords(sentenceIdx) + 1;  // +1 for prefix gap
  }

  assert(offset < sourceTokenTags.size());
  targetTokenTags.push_back(sourceTokenTags[offset]);  // token_tag for ending whitespace
}

AnnotatedText restoreSource(AnnotatedText const &in, std::vector<HTML::Taint> &token_tags,
                            std::vector<HTML::Span>::const_iterator span_it,
                            std::vector<HTML::Span>::const_iterator span_end) {
  auto prev_it = span_it;  // safe because first span is always empty span, and
                           // and the while-loop below will do the rest

  // workspace variables for lambda
  std::string html;
  HTML::Taint opening, closing;

  return apply(in, [&](ByteRange range, string_view token, bool last) {
    // Do encoding of any entities that popped up in the translation
    // (Also effectively clears html from previous call)
    encodeEntities(token, html);

    size_t offset = 0;  // Size added by prepending HTML
    size_t whitespace_size = countPrefixWhitespaces(token);

    // Close tags we want to show up left (before) the token, but open tags
    // ideally come directly after any prefix whitespace. However, some tokens
    // match multiple spans. If a previous span has added an open tag, after any
    // whitespace, and the next span closes said tag again, we need to close
    // it after the whitespace. So after the first open tag, any closing tag
    // should also align right, after whitespace, not before. Hence this bool.
    bool close_left = true;

    // Potential issue: spans and tokens can intersect, e.g.
    //
    //    text  <p> h <u> e </u> ll o </p>
    //   spans     |1|   |2|    |3333| (so only 2 is tainted with <p><u>, others only <p>)
    //  tokens     |111111111111111|2|
    //
    // Now 1 covers span 1 to 3, so what taint should it get? Just <p>, or <p><u>?

    // Seek to the last span that overlaps with this token
    while (true) {
      diffTags(prev_it->tags, span_it->tags, opening, closing);
      prev_it = span_it;

      for (auto cit = closing.crbegin(); cit != closing.crend(); ++cit) {
        std::string close_tag = format("</{}>", (*cit)->name);
        html.insert(offset + (close_left ? 0 : whitespace_size), close_tag);
        offset += close_tag.size();
      }

      for (HTML::Tag const *tag : opening) {
        std::string open_tag = format("<{}{}>", tag->name, tag->attributes);
        html.insert(offset + whitespace_size, open_tag);
        offset += open_tag.size();
        close_left = false;
      }

      if (span_it + 1 != span_end && ((span_it + 1)->begin < range.end || last)) {
        span_it++;
        continue;
      }

      break;
    }

    // TODO: This is just the taint of the last span, not the ones in between.
    // This makes us lose empty tags, and maybe some markup as well, in the
    // response target HTML restoration.
    token_tags.push_back(prev_it->tags);

    return html;
  });
}

AnnotatedText restoreTarget(AnnotatedText const &in, std::vector<HTML::Taint> const &token_tags_target) {
  auto token_prev_it = token_tags_target.begin();
  auto token_tags_it = token_tags_target.begin() + 1;

  // workspace for lambda
  std::string html;
  HTML::Taint opening, closing;

  AnnotatedText out = apply(in, [&](ByteRange range, string_view token, bool last) {
    // Do encoding of any entities that popped up in the translation
    // (Also effectively clears html from previous call)
    encodeEntities(token, html);

    size_t offset = 0;  // Size added by prepending HTML
    size_t whitespace_size = countPrefixWhitespaces(token);

    assert(token_tags_it != token_tags_target.end());
    diffTags(*token_prev_it, *token_tags_it, opening, closing);

    for (auto cit = closing.crbegin(); cit != closing.crend(); ++cit) {
      std::string close_tag = format("</{}>", (*cit)->name);
      html.insert(offset, close_tag);
      offset += close_tag.size();
    }

    for (HTML::Tag const *tag : opening) {
      std::string open_tag = format("<{}{}>", tag->name, tag->attributes);
      html.insert(offset + whitespace_size, open_tag);
      offset += open_tag.size();
    }

    // If this is the last token of the response, close all open tags.
    if (last) {
      for (auto cit = token_tags_it->crbegin(); cit != token_tags_it->crend(); ++cit) {
        html += format("</{}>", (*cit)->name);
      }
    }

    ++token_prev_it;
    ++token_tags_it;

    return html;
  });

  // Assert that we did in fact use all our taints
  assert(token_tags_it == token_tags_target.end());

  return out;
}

std::ostream &debugPrintMapping(std::ostream &out, Response const &response,
                                std::vector<std::vector<size_t>> const &alignments,
                                std::vector<HTML::Taint> const &token_tags_target) {
  auto taints = token_tags_target.begin();
  for (size_t sentenceIdx = 0; sentenceIdx < response.target.numSentences(); ++sentenceIdx) {
    out << "Mapped sentence prefix with tags: ";
    for (auto &&taint : *(++taints)) out << '/' << taint->name;
    out << '\n';

    for (size_t wordIdx = 0; wordIdx < response.target.numWords(sentenceIdx); ++wordIdx) {
      assert(sentenceIdx < alignments.size());
      assert(wordIdx < alignments[sentenceIdx].size());

      out << "Mapped ";
      out << std::setw(10) << std::setfill(' ') << response.target.word(sentenceIdx, wordIdx);
      out << " to ";
      out << std::setw(10) << std::setfill(' ') << response.source.word(sentenceIdx, alignments[sentenceIdx][wordIdx]);
      out << " with tags: ";
      for (auto &&taint : *(++taints)) out << '/' << taint->name;
      out << '\n';
    }
  }

  out << "Mapped end-of-input with tags: ";
  for (auto &&taint : *(++taints)) out << '/' << taint->name;
  out << '\n';

  assert(++taints == token_tags_target.end());
  return out;
}

std::ostream &debugPrintAlignmentScores(std::ostream &out, Response const &response) {
  out << "std::vector<std::vector<std::vector<float>>> alignments{\n";
  for (size_t sentenceIdx = 0; sentenceIdx < response.source.numSentences(); ++sentenceIdx) {
    out << "  {\n";
    for (size_t t = 0; t < response.alignments[sentenceIdx].size(); ++t) {
      out << "    {";
      for (size_t s = 0; s < response.alignments[sentenceIdx][t].size(); ++s) {
        out << std::fixed << std::setw(8) << std::setprecision(8) << std::setfill(' ')
            << response.alignments[sentenceIdx][t][s];
        out << ", ";
      }
      out << "},\n";
    }
    out << "  },\n";
  }
  return out << "};\n";
}

size_t debugCountTokens(AnnotatedText const &text) {
  size_t tokens = 1;  // for the ending gap
  for (size_t sentenceIdx = 0; sentenceIdx < text.numSentences(); ++sentenceIdx) {
    tokens += 1 + text.numWords(sentenceIdx);  // pre-sentence prefix/gap + each word
  }
  return tokens;
}

}  // namespace

namespace marian::bergamot {

HTML::HTML(std::string &&source, bool process_markup) {
  if (!process_markup) return;
  std::string original = std::move(source);
  markup::instream in(original.data(), original.data() + original.size());
  markup::Scanner scanner(in);
  source.clear();  // source is moved out of, so should be clear anyway

  Tag *tag;
  Taint stack;
  spans_.push_back(Span{0, 0, {}});

  bool stop = false;
  while (!stop) {
    switch (scanner.next()) {
      case markup::Scanner::TT_ERROR:
        throw BadHTML("HTML parse error");

      case markup::Scanner::TT_EOF:
        stop = true;
        break;

      case markup::Scanner::TT_TEXT: {
        auto begin = source.size();
        source.append(scanner.value());
        spans_.push_back(Span{begin, source.size(), stack});
        filterEmpty(stack);
      } break;

      case markup::Scanner::TT_TAG_START:
        // If it makes sense to treat this element as a break in a word (e.g.
        // <br>, <img>, <li>) make sure it does so in this text as well.
        // TODO: Strong assumption here that the language uses spaces to
        // separate words
        if (isBlockElement(scanner.tag()) && !source.empty() && source.back() != ' ') source.push_back(' ');

        // pool_ takes ownership of our tag, makes sure it's freed when necessary
        pool_.emplace_back(new Tag{std::string(scanner.tag()), std::string(), isVoidTag(scanner.tag())});

        // Tag *tag is used by attribute parsing
        tag = pool_.back().get();

        stack.push_back(tag);

        // Empty elements (e.g. <img>) are not applicable to a span of text
        // so instead we "apply" them to an empty span in between, and then
        // immediately remove them again from the stack.
        if (tag->empty) {
          spans_.push_back(Span{source.size(), source.size(), stack});
          stack.pop_back();
        }
        break;

      case markup::Scanner::TT_TAG_END:
        // Note: self-closing tags emit TT_TAG_END immediately after TT_TAG_START
        // but since we're parsing HTML5, a sole <img> will never emit a TT_TAG_END
        if (stack.empty()) throw BadHTML(format("Encountered more closing tags ({}) than opening tags", scanner.tag()));

        if (stack.back()->name != scanner.tag())
          throw BadHTML(format("Encountered unexpected closing tag </{}>, stack is {}", scanner.tag(), stack));

        // What to do with "<u></u>" case, where tag is immediately closed
        // so it never makes it into the taint of any of the spans? This adds
        // an empty span so it still gets recorded in spans_.
        if (spans_.empty() || !containsTag(spans_.back().tags, stack.back()))
          spans_.push_back(Span{source.size(), source.size(), stack});

        stack.pop_back();
        break;

      case markup::Scanner::TT_ATTRIBUTE:
        assert(tag != nullptr);
        tag->attributes += format(" {}=\"{}\"", scanner.attribute(), scanner.value());
        break;

      default:
        break;
    }
  }

  if (!stack.empty()) throw BadHTML(format("Not all tags were closed: {}", stack));

  // Add a trailing span (that's empty) to signify all closed tags.
  spans_.emplace_back(Span{source.size() + 1, source.size() + 1, stack});
}

void HTML::restore(Response &response) {
  // No-op if process_markup was false (and thus spans_ is empty)
  // TODO: replace this with optional<HTML> at a higher level
  if (spans_.empty()) return;

  // We need alignment info to transfer the HTML tags from the input to the
  // translation. If those are not available, no HTML in translations for you.
  ABORT_UNLESS(hasAlignments(response),
               "Response object does not contain alignments. TranslationModel or ResponseOptions is misconfigured?");

  // Reconstruction of HTML tags:
  // 1. Map each token to a Span
  // 2. Apply the taint of that span to the token
  // 3. Reconstruct the source HTML with these tainted tokens
  // 4. Transfer the taint from the source tokens to the target tokens using alignment information
  // 5. Reconstruct the target HTML with these tainted tokens

  std::vector<Taint> token_tags;  // List of HTML tags active per token in source
                                  // Calculating these is a side-effect of restoring
                                  // the HTML in response.source.

  AnnotatedText source = restoreSource(response.source, token_tags, spans_.cbegin(), spans_.cend());
  assert(token_tags.size() == debugCountTokens(response.source));

  // Find for every token in target the token in source that best matches.
  std::vector<std::vector<size_t>> alignments;
  hardAlignments(response, alignments);

  std::vector<Taint> token_tags_target;
  token_tags_target.emplace_back();  // add empty one to the beginning for easy
                                     // life later on (we start iterating at 1,
                                     // and can then do i - 1 for empty.
  copyTaint(response, alignments, token_tags, token_tags_target);
  assert(token_tags_target.size() == debugCountTokens(response.target) + 1);

  // DebugPrintMapping(std::cerr, response, alignments, token_tags_target);

  AnnotatedText target = restoreTarget(response.target, token_tags_target);

  response.source = source;
  response.target = target;
}

}  // namespace marian::bergamot