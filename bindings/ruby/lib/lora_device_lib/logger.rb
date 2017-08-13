require 'logger'

module LoraDeviceLib

    class Logger
    
        @logger = ::Logger.new(STDOUT)
        
        def self.error(msg)
            @logger.error(msg)
        end
        
        def self.info(msg)
            @logger.info(msg)
        end
        
        def self.debug(msg)
            @logger.debug(msg)
        end
        
        
    
    end

end
