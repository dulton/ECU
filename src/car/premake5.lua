group("src")
project("ECU-car")
uuid("6a0db7a6-2e21-43b1-b0fb-d158b005456c")
kind("StaticLib")
language("C++")

files({
      "ers.h",
      "ers.cpp",
      "balance.h",
      "balance.cpp",
      "tire.cpp",
      "tire.h",
})
links({
      "irsdk",
})
