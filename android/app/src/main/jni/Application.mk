
# Uncomment this if you're using STL in your project
# See CPLUSPLUS-SUPPORT.html in the NDK documentation for more information
APP_STL := c++_shared
APP_PLATFORM = android-10
#APP_ABI := armeabi-v7a
APP_ABI :=  all
APP_MODULES := fastlz-wrapper
APP_CPPFLAGS += -frtti 
APP_CPPFLAGS += -fexceptions
