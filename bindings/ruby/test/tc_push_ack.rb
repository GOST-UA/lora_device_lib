require 'minitest/autorun'
require 'ldl'

class TestPushAck < Minitest::Test

    include LDL

    def setup
        @state = Semtech::PushAck.new
    end

    def test_encode_default    
        
        out = @state.encode    
        
        iter = out.unpack("CS>Ca8").each
        
        assert_equal Semtech::Message::VERSION, iter.next
        iter.next
        
        assert_equal @state.class.type, iter.next
        assert_equal "\x00\x00\x00\x00\x00\x00\x00\x00", iter.next        
        
    end
    
    def test_decode_default
        
        input = @state.encode
        
        input = "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00{\"rxpk\":[]}"
        
        decoded = Semtech::PushData.decode(input)
        
        assert_equal 0,  decoded.token
        assert_equal EUI64.new("00-00-00-00-00-00-00-00"), decoded.eui
        
    end

end
