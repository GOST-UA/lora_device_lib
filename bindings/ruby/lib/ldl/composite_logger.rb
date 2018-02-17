require 'logger'

module LDL

    class CompositeLogger

        @loggers = []
        
        def self.<<(logger)
            @loggers << logger
        end
    
        def self.error(*args, &block)
            @loggers.each do |logger|
                logger.error(*args, &block)
            end
        end
        
        def self.info(*args, &block)
            @loggers.each do |logger|
                logger.info(*args, &block)
            end
        end
        
        def self.debug(*args, &block)
            @loggers.each do |logger|
                logger.debug(*args, &block)
            end
        end
    
    end

end
