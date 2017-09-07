require 'minitest/autorun'
require 'ldl'

class TestTXPacket < Minitest::Test

    include LDL

    def setup
        @state = Semtech::TXPacket.new
    end

    def test_to_json_default
        output = @state.to_json
    end

    def test_from_json
    end
    
end
