#!/usr/bin/env bash
# Runs scripts/check-version.sh against a copy of the repo's version-bearing
# files, breaking one file per case.
set -u

repo=$(cd "$(dirname "$0")/../.." && pwd)
check="$repo/scripts/check-version.sh"
files=(
  CMakeLists.txt
  packaging/rpm/flameshot.spec
  packaging/debian/changelog
  snapcraft.yaml
  data/appdata/org.flameshot.Flameshot.metainfo.xml
)
failures=0

# run_case <name> <file to break or ""> <sed expression>
run_case() {
  local name=$1 file=$2 expr=$3
  local tmp out status
  tmp=$(mktemp -d)
  for f in "${files[@]}"; do
    mkdir -p "$tmp/$(dirname "$f")"
    cp "$repo/$f" "$tmp/$f"
  done
  [ -n "$file" ] && sed -i "$expr" "$tmp/$file"

  out=$("$check" "$tmp" 2>&1)
  status=$?
  rm -rf "$tmp"

  if [ -z "$file" ]; then
    if [ $status -ne 0 ]; then
      echo "FAIL $name: expected exit 0, got $status"$'\n'"$out"
      failures=$((failures + 1))
      return
    fi
  elif [ $status -eq 0 ] || ! grep -qF "$file" <<<"$out"; then
    echo "FAIL $name: expected non-zero exit naming $file, got $status"$'\n'"$out"
    failures=$((failures + 1))
    return
  fi
  echo "ok   $name"
}

run_case "consistent files pass" "" ""
run_case "rpm Version mismatch" packaging/rpm/flameshot.spec \
  's/^Version: .*/Version: 0.0.0.0/'
run_case "rpm changelog mismatch" packaging/rpm/flameshot.spec \
  '0,/^\* .* - [0-9.]*$/s/ - [0-9.]*$/ - 0.0.0.0/'
run_case "debian changelog mismatch" packaging/debian/changelog \
  '1s/(.*-/(0.0.0.0-/'
run_case "snapcraft mismatch" snapcraft.yaml \
  "s/^version: .*/version: '0.0.0.0'/"
run_case "metainfo mismatch" data/appdata/org.flameshot.Flameshot.metainfo.xml \
  '0,/<release version="[^"]*"/s/<release version="[^"]*"/<release version="0.0.0.0"/'

[ $failures -eq 0 ]
