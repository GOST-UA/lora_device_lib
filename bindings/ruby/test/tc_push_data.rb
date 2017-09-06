require 'minitest/autorun'
require 'ldl'

class TestPushData < Minitest::Test

    include LDL::Semtech

    def setup
        @state = PushData.new
    end

    def test_encode_default    
        
        out = @state.encode    
        
        iter = out.unpack("CS>Ca8a*").each
        
        assert_equal Message::VERSION, iter.next
        iter.next
        
        assert_equal @state.class.type, iter.next
        assert_equal "\x00\x00\x00\x00\x00\x00\x00\x00", iter.next        
        
        assert_equal({"rxpk" => []}, JSON.parse(iter.next))
        
    end
    
    def test_decode_default
        
        input = @state.encode
        
        input = "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00{\"rxpk\":[]}"
        
        decoded = PushData.decode(input)
        
        assert_equal 0,  decoded.token
        assert_equal LDL::EUI64.new("00-00-00-00-00-00-00-00"), decoded.eui
        assert_nil decoded.stat
        
    end

end
