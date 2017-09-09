require 'minitest/autorun'
require 'ldl'

class TestPushData < Minitest::Test

    include LDL

    def test_encode_default    
        
        state = Semtech::PushData.new
        
        out = state.encode    
        
        iter = out.unpack("CS>Ca8a*").each
        
        assert_equal Semtech::Message::VERSION, iter.next
        iter.next
        
        assert_equal state.class.type, iter.next
        assert_equal "\x00\x00\x00\x00\x00\x00\x00\x00", iter.next        
        
        assert_equal({"rxpk" => []}, JSON.parse(iter.next))
        
    end
    
    def test_decode_default
        
        input = "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00{\"rxpk\":[]}"
        
        decoded = Semtech::PushData.decode(input)
        
        assert_equal 0,  decoded.token
        assert_equal EUI64.new("00-00-00-00-00-00-00-00"), decoded.eui
        assert_nil decoded.stat
        
    end
    
    def test_encode_decode
        
        eui = EUI64.new("EA-15-79-3E-11-2C-7F-93")
        
        state = Semtech::PushData.new(
            eui: eui
        )
        
        decoded = Semtech::PushData.decode(state.encode)
        
        assert_kind_of Semtech::PushData, decoded
        assert_equal eui, decoded.eui
        assert_nil decoded.stat
        
    end

end
