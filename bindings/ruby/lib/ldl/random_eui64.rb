require 'securerandom'

module LDL

    class RandomEUI64
    
        def self.eui
            
            bytes = SecureRandom.random_bytes 8
            
            bytes.bytes[0] |= 0x2
            
            EUI64.new bytes
            
        end
        
    end

end
