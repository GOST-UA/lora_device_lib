require 'minitest/autorun'
require 'ldl'

class TestPullResp < Minitest::Test

    include LDL

    def setup
        @state = Semtech::PullResp.new
    end

    def test_encode_default    
        
        out = @state.encode    
        
        iter = out.unpack("CS>Ca*").each
        
        assert_equal Semtech::Message::VERSION, iter.next
        iter.next
        
        assert_equal @state.class.type, iter.next
        
        JSON.parse iter.next
        
    end
    
    def test_decode_default
        
        input = @state.encode
        
        input = "\x02\x00\x00\x03#{{:txpk => Semtech::TXPacket.new}.to_json}"
        
        decoded = Semtech::PullResp.decode(input)
        
        assert_equal 0, decoded.token
        
    end
    
    def test_encode_decode        
        assert_kind_of Semtech::PullResp, Semtech::Message.decode(@state.encode)        
    end

end
