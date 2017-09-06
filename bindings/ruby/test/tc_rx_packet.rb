require 'minitest/autorun'
require 'ldl'

class TestRXPacket < Minitest::Test

    include LDL::Semtech
    
    def setup
        @state = RXPacket.new
    end

    def test_to_json_default
        @state.to_json
    end

    def test_from_json
    end
    
end
