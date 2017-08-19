require 'minitest/autorun'
require 'lora_device_lib'

class TestFrame < Minitest::Test
    
    include LoraDeviceLib

    def test_unconfirmedUp
    
        msg = UnconfirmedUp.new        
        decoded = Frame.decode(msg.encode)
        
    end
    
    def test_joinReq
    
        msg = JoinReq.new
        decoded = Frame.decode(msg.encode)
        
    end
    
    def test_joinAccept
    
        msg = JoinAccept.new
        decoded = Frame.decode(msg.encode)
        
    end

end
