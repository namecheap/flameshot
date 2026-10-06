#!/usr/bin/env bash
# Fails when a packaging file's hand-written version differs from
# FLAMESHOT_VERSION in CMakeLists.txt.
# Usage: check-version.sh [repo-root]
set -u

root=${1:-$(git rev-parse --show-toplevel)}
cd "$root" || exit 2

expected=$(sed -n 's/^set(FLAMESHOT_VERSION \([0-9.]*\))$/\1/p' CMakeLists.txt)
if [ -z "$expected" ]; then
  echo "CMakeLists.txt: FLAMESHOT_VERSION not found"
  exit 2
fi
status=0

# check <file> <description> <found version>
check() {
  if [ "$3" != "$expected" ]; then
    echo "$1: $2 is '$3', expected '$expected'"
    status=1
  fi
}

spec=packaging/rpm/flameshot.spec
check $spec "Version" "$(sed -n 's/^Version: *//p' $spec)"
check $spec "newest %changelog entry" \
  "$(sed -n 's/^\* .* - \([0-9.]*\)$/\1/p' $spec | head -1)"

deb=packaging/debian/changelog
check $deb "newest entry" "$(sed -n '1s/^flameshot (\(.*\)-[^-]*).*/\1/p' $deb)"

snap=snapcraft.yaml
check $snap "version" "$(sed -n "s/^version: '\(.*\)'$/\1/p" $snap)"

meta=data/appdata/org.flameshot.Flameshot.metainfo.xml
check $meta "newest <release>" \
  "$(sed -n 's/.*<release version="\([^"]*\)".*/\1/p' $meta | head -1)"

[ $status -eq 0 ] && echo "All packaging versions match $expected"
exit $status
