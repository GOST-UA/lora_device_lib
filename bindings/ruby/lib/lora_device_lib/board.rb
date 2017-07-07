module LoraDeviceLib

    class Board
        
        attr_reader :type
        
        # @param state [true,false] true if chip is selected
        def select(state)
        
            @select = state
            
            if state
                puts "board select held"
            else
                puts "board select released"
            end
        end
        
        # @param state [true, false] true if reset line driven
        def reset(state)
        
            @reset = state
        
            if state
                puts "board reset held"
            else
                puts "board reset released"
            end
        end
        
        # wait for the minimum time required for a reset line to take effect
        def reset_wait
            puts "waiting"
        end
        
        # @param data [Integer] an integer in range (0..255)
        def write(data)
        
            @write_buffer << data.chr
        
            puts "wrote #{"0x%02X" % data}"
            
        end
        
        # @return [Integer] integer in range (0..255)
        def read        
            puts "read (dummy 0x00)"
            0
        end
        
        def initialize
            @write_buffer = ""
            @reset = false
            @state = false
        end
    
    end

end
