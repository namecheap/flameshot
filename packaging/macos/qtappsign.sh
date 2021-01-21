#!/bin/bash
# Inspired by
# https://localazy.com/blog/how-to-automatically-sign-macos-apps-using-github-actions
# https://forum.qt.io/topic/96652/how-to-notarize-qt-application-on-macos/18

APP_NAME="${1}"
TEMP_CI_CERT_FILENAME="temp_ci_appleDistribution.p12"

# Get the following variables from secrets via envs
# APPLE_DEV_IDENTITY
# APPLE_DEV_USER
# APPLE_DEV_PASS
# APPLE_DEVELOPER_ID_APPLICATION_CERT_PASS
# APPLE_DEVELOPER_ID_APPLICATION_CERT_DATA
# APPLE_TEMP_CI_KEYCHAIN_PASS

echo "*** TEMP_CI_CERT_FILENAME: $APPLE_TEMP_CI_KEYCHAIN_PASS"
echo "*** APPLE_DEV_USER: ${APPLE_DEV_USER}"

echo "--> Create key-chain and import certificate"

# ***************************************************
# ***** Create key-chain and import certificate *****
# ***************************************************
# create keychain
security create-keychain -p "${APPLE_TEMP_CI_KEYCHAIN_PASS}" build.keychain
security default-keychain -s build.keychain
security unlock-keychain -p "${APPLE_TEMP_CI_KEYCHAIN_PASS}" build.keychain

# import certificate
[ -r "${TEMP_CI_CERT_FILENAME}" ] && rm ${TEMP_CI_CERT_FILENAME}
echo "${APPLE_DEVELOPER_ID_APPLICATION_CERT_DATA}" | base64 --decode > "${TEMP_CI_CERT_FILENAME}"
security import "${TEMP_CI_CERT_FILENAME}" -P "${APPLE_DEVELOPER_ID_APPLICATION_CERT_PASS}" -k build.keychain -T /usr/bin/codesign
[ -r "${TEMP_CI_CERT_FILENAME}" ] && rm ${TEMP_CI_CERT_FILENAME}
security find-identity -v
security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k "${APPLE_TEMP_CI_KEYCHAIN_PASS}" build.keychain

# ****************************
# ***** Sign application *****
# ****************************
echo "--> Start application signing process"
codesign --sign "${APPLE_DEV_IDENTITY}" --verbose --deep ${APP_NAME}.app

# *****************************
# ***** Build dmg package *****
# *****************************
echo "--> Start packaging process"
"$(brew --prefix qt5)/bin/macdeployqt" "${APP_NAME}.app" -dmg -sign-for-notarization="${APPLE_DEV_IDENTITY}"

#echo "--> Update dmg package links"
#./{HELPERS_SCRIPTS_PATH}/update_package.sh

# ********************************
# ***** Notarize application *****
# ********************************
echo "--> Start Notarization process"
response=$(xcrun altool -t osx -f "${APP_NAME}.dmg" --primary-bundle-id "org.namecheap.${APP_NAME}" --notarize-app -u "${APPLE_DEV_USER}" -p "${APPLE_DEV_PASS}")
requestUUID=$(echo "${response}" | tr ' ' '\n' | tail -1)

while true; do
  echo "--> Checking notarization status"
  statusCheckResponse=$(xcrun altool --notarization-info "${requestUUID}" -u "${APPLE_DEV_USER}" -p "${APPLE_DEV_PASS}")

  isSuccess=$(echo "${statusCheckResponse}" | grep "success")
  isFailure=$(echo "${statusCheckResponse}" | grep "invalid")

  if [[ "${isSuccess}" != "" ]]; then
    echo "Notarization done!"
    xcrun stapler staple "${APP_NAME}.dmg"
    echo "Stapler done!"
    break
  fi
  if [[ "${isFailure}" != "" ]]; then
    echo "${statusCheckResponse}"
    echo "Notarization failed"
    exit 1
  fi
  echo "Notarization not finished yet, sleep 2m then check again..."
  sleep 120
done

# ****************************
# ***** Sign dmg package *****
# ****************************
echo "--> Start dmg signing process"
codesign --sign "${APPLE_DEV_IDENTITY}" --verbose --deep "${APP_NAME}.dmg"

# ***********************************
# ***** Verify dmg package sign *****
# ***********************************
echo "--> Start verify signing process"
codesign -dv --verbose=4 "${APP_NAME}.dmg"
