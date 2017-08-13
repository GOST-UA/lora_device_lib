require 'minitest/autorun'
require 'lora_device_lib/ext_frame'

class TestFrame < Minitest::Test
    
    include LoraDeviceLib
    
    DUMMY_KEY = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

    def test_unconfirmedUp
    
        msg = UnconfirmedUp.new(
            appSKey: DUMMY_KEY,
            nwkSKey: DUMMY_KEY)
            
        out = msg.encode
        
        puts out.bytes.map{|c|02%X % c.to_s(16)}.join("\\x")
    
    end

end
