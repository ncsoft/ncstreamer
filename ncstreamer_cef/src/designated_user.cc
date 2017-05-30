/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/designated_user.h"

#include <codecvt>
#include <locale>

#include "ncstreamer_cef/src/local_storage.h"
#include "ncstreamer_cef/src/streaming_service.h"


namespace ncstreamer {
void DesignatedUser::SetUp(const std::wstring &designated_user) {
  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

  const auto &current_user = converter.to_bytes(designated_user);
  const auto &prev_user = LocalStorage::Get()->GetDesignatedUser();
  if (current_user == prev_user) {
    return;
  }

  StreamingService::Get()->LogOutAll();
  LocalStorage::Get()->SetDesignatedUser(current_user);
}
}  // namespace ncstreamer
