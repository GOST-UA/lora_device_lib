require 'logger'

module LoraDeviceLib

    class Logger
    
        @logger = ::Logger.new('ldl')
        
        def self.error(msg)
            @logger.error(msg)
        end
        
        def self.info(msg)
            @logger.info(msg)
        end
    
    end

end
