import re
import sys

# - tries to determine -march, -mfpu, -mcpu flags
# - reads /proc/cpuinfo
# - fails when NEON is not present
# - fails when CPU is not from ARM (i.e: Apple M1)
# by code@kroket.io

# https://gist.github.com/fm4dd/c663217935dc17f0fc73c9c81b0aa845
# https://community.arm.com/arm-community-blogs/b/tools-software-ides-blog/posts/compiler-flags-across-architectures-march-mtune-and-mcpu
lookup = {
    0xc05: {"cpu": "cortex-a5", 'arch': "armv7-a", 'fpu': 'neon-fp16'},
    0xc07: {"cpu": "cortex-a7", 'arch': "armv7ve", 'fpu': 'neon-vfpv4'},
    0xC08: {"cpu": "cortex-a8", 'arch': "armv7-a+neon", 'fpu': 'neon'},
    0xC09: {"cpu": "cortex-a9", 'arch': "armv7-a+neon", 'fpu': 'neon-fp16'},
    0xc0f: {"cpu": "cortex-a15", 'arch': "armv7ve", 'fpu': 'neon-vfpv4'},
    0xc0e: {"cpu": "cortex-a17", 'arch': "armv7-a", 'fpu': 'neon'},
    0xd03: {"cpu": "cortex-a53", 'arch': "armv8-a+crc", 'fpu': 'crypto-neon-fp-armv8'},
    0xd07: {"cpu": "cortex-a57", 'arch': "armv8-a+crc", 'fpu': 'neon-fp-armv8'},
    0xd08: {"cpu": "cortex-a72", 'arch': "armv8-a+crc", 'fpu': 'crypto-neon-fp-armv8'},
    0xd09: {"cpu": "cortex-a73", 'arch': "armv8-a+crc", 'fpu': 'neon'},
    0xd0a: {"cpu": "cortex-a73", 'arch': "armv8-a+crc", 'fpu': 'neon'},
    0xd0b: {"cpu": "cortex-a76", 'arch': "armv8.2-a", 'fpu': 'neon'},
    0xd0d: {"cpu": "cortex-a77", 'arch': "armv8.2-a", 'fpu': 'neon'},
    0xd47: {"cpu": "cortex-a710", 'arch': "armv9-a", 'fpu': 'neon'},
    0xb76: {"cpu": "arm1176jzf-s", 'arch': "armv6", 'fpu': 'neon'},
    0xd04: {"cpu": "cortex-a35", 'arch': "armv8-a+crc", 'fpu': 'neon'},
    0xd05: {"cpu": "cortex-a55", 'arch': "armv8.2-a", 'fpu': 'neon'},
    0xd46: {"cpu": "cortex-a510", 'arch': "armv9-a", 'fpu': 'neon'}
}

f = open("/proc/cpuinfo", "r")
lines = f.read()
f.close()

if "neon" not in lines:
    raise Exception("NEON not found")

# establish cortex version
match = re.search(r"^CPU part.*: ([0-9A-Za-z]+)", lines, re.MULTILINE)
if not match:
    raise Exception("Could not establish CPU part")


version = int(match.group(1), 16)
if version not in lookup:
    raise Exception("unknown machine")

item = lookup[version]
msg = f"{item}"
sys.stdout.write(f"-march={item['arch']} -mcpu={item['cpu']} -mfpu={item['fpu']}")
