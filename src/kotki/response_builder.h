#ifndef SRC_BERGAMOT_RESPONSE_BUILDER_H_
#define SRC_BERGAMOT_RESPONSE_BUILDER_H_

#include <optional>

#include "marian-lite/data/types.h"
#include "kotki/quality_estimator.h"
#include "kotki/response.h"
#include "kotki/response_options.h"
#include "kotki/vocabs.h"

// For now we will work with this, to avoid complaints another structure is hard
// to operate with.

namespace marian {
namespace bergamot {

/// ResponseBuilder is a callback functor. It is expected to be bound to a
/// Request after giving it the context of options, vocabs and promise to set.
/// It constructs the Response and it's members based on options
/// (quality=on|off, alignments=on|off, mappings=on|off, splitmode=sentence |
/// paragraph).

class ResponseBuilder {
 public:
  /// @param [in] responseOptions: ResponseOptions, indicating what to include
  /// or not in the response and any additional configurable parameters.
  /// @param [in] vocabs: marian vocab object (used in decoding)
  /// @param [in] qualityEstimator: the QualityEstimator model that can be used
  /// to provide translation quality probability.
  ResponseBuilder(AnnotatedText &&source, const Vocabs &vocabs, const QualityEstimator &qualityEstimator)
      : source_(std::move(source)),
        vocabs_(vocabs),
        qualityEstimator_(qualityEstimator) {}

  Response build(Histories &&histories) {
    ABORT_IF(source_.numSentences() != histories.size(), "Mismatch in source and translated sentences");
    Response response;

    // Move source_ into response.
    response.source = std::move(source_);

    // Should be after source is set
    buildTranslatedText(histories, response);
    return response;
  }

 private:
  /// Builds qualityScores from histories and writes to response. expects
  /// buildTranslatedText to be run before to be able to obtain target text and
  /// subword information.
  /// @param histories [in]
  /// @param response [out]
  void buildQualityScores(Histories &histories, Response &response);

  /// Builds alignments from histories and writes onto response.
  /// @param histories [in]
  /// @param response [out]
  void buildAlignments(Histories &histories, Response &response);

  /// Builds translated text and subword annotations and writes onto response.
  /// @param histories [in]
  /// @param response [out]
  void buildTranslatedText(Histories &histories, Response &response);

  // Data members are context/curried args for the functor.

  const Vocabs &vocabs_;                       // vocabs are required for decoding
                                               // and any source validation checks.
  AnnotatedText source_;

  const QualityEstimator &qualityEstimator_;
};
}  // namespace bergamot
}  // namespace marian

#endif  //  SRC_BERGAMOT_RESPONSE_BUILDER_H_
