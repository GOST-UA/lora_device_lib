require 'logger'

module LDL

    class Logger

        @loggers = []
        
        def self.<<(logger)
            @loggers << logger
        end
    
        def self.error(msg)
            @loggers.each do |logger|
                logger.error(msg)
            end
        end
        
        def self.info(msg)
            @loggers.each do |logger|
                logger.info(msg)
            end
        end
        
        def self.debug(msg)
            @loggers.each do |logger|
                logger.debug(msg)
            end
        end
    
    end

end
