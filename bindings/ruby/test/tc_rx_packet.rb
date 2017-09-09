require 'minitest/autorun'
require 'ldl'

class TestRXPacket < Minitest::Test

    include LDL

    def setup
        @state = Semtech::RXPacket.new
    end

    def test_to_json_default
        @state.to_json
    end

    def test_from_h
        input = {}        
        Semtech::RXPacket.from_h(input)
    end
    
end
