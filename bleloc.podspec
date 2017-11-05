Pod::Spec.new do |s|
  s.name         = "bleloc"
  s.version      = "1.1"
  s.summary      = "Localization library"
  s.homepage     = "https://github.com/realCabot/"
  s.description  = <<-DESC
This is a localization library for bluetooth le beacons.
                   DESC

  s.license      = "MIT"
  s.author    = "realCabot"
  s.source       = { :http => "http://github.com/RealCabot/blelocpp/releases/download/1.1/bleloc.framework.zip" }
  s.preserve_path = "bleloc.framework"
  s.vendored_frameworks = "bleloc.framework"
  s.platforms = {:ios => "8.4"}
  s.requires_arc = false
  s.ios.frameworks = 'AVFoundation', 'AssetsLibrary', 'CoreMedia'
end
