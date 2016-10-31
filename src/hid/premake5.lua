
group("src")
project("ECU-hid")
  uuid("d4118174-efd3-433c-9ac2-237a4275aa69")
  kind("StaticLib")
  language("C++")
  files({
    "hid.cpp",
    "hid.h",
  })
  links({
    "dinput8",
    "dxguid",
  })