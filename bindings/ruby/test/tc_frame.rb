require 'minitest/autorun'
require 'lora_device_lib'

class TestFrame < Minitest::Test
    
    include LoraDeviceLib

    def test_unconfirmedUp
    
        msg = UnconfirmedUp.new
            
        out = msg.encode
        
        puts out.bytes.map{|c|"\\x%02X" % c}.join("")
    
    end
    
    def test_joinReq
    
        msg = JoinReq.new
            
        out = msg.encode
        
        puts out.bytes.map{|c|"\\x%02X" % c}.join("")
    
    end
    
    def test_joinAccept
    
        msg = JoinAccept.new
        
        out = msg.encode
        
        puts out.bytes.map{|c|"\\x%02X" % c}.join("")
    
    end

end
