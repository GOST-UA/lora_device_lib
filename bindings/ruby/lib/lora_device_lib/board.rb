module LDL

    class Board
    
        # @param state [true,false] true if chip is selected
        def select(state)
            puts "board select: #{state}"
        end
        
        # @param state [true, false] true if reset line driven
        def reset(state)
            puts "board reset: #{state}"
        end
        
        # wait for the minimum time required for a reset line to take effect
        def reset_wait
            puts "waiting for board reset"
        end
        
        # @param data [Integer] an integer in range (0..255)
        def write(data)
            puts "wrote #{"0x%02X" % data}"
        end
        
        # @return [Integer] integer in range (0..255)
        def read        
            puts "read 0x00"
            0
        end
    
    end

end
