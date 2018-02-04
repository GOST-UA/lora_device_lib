require 'minitest/autorun'
require 'ldl'
require 'socket'
require 'timeout'

class TestMAC < Minitest::Test

    include LDL

    attr_reader :state
    attr_reader :s
    attr_reader :broker
    
    def setup
    
        Thread.abort_on_exception = true
   
        LDL.const_set 'SystemTime', TimeSource.new
        SystemTime.start
    
        @broker = Broker.new    
    
        @state = MAC.new broker
    
        sleep 0.1
    
    end
    
    def test_join
    
        assert_raises JoinTimeout do
    
            state.join
            
        end
    
    end
    
    def teardown    
    
        SystemTime.stop
        
        LDL.send(:remove_const, :SystemTime)
        
        Thread.abort_on_exception = false
    end
    
end
