require 'logger'

module LDL

    LOG_FORMATTER = Proc.new do |severity, datetime, progname, msg|
        "#{severity.ljust(5)} [#{datetime.strftime("%Y-%m-%d %H:%M:%S")}] #{msg}\n"
    end

    module LoggerMethods
    
        def log_info(msg)
        
            if LDL.const_defined? "LDL::Logger"
                LDL::Logger.info { "#{SystemTime.time.to_s.rjust(10)} #{msg}" } 
            end
        end
        
        def log_error(msg)
            if LDL.const_defined? "LDL::Logger"
                LDL::Logger.error { "[#{SystemTime.time.to_s.rjust(10)}] #{msg}" } 
            end                
        end
        
        def log_debug(msg)
            if LDL.const_defined? "LDL::Logger"
                LDL::Logger.debug { "[#{SystemTime.time.to_s.rjust(10)}] #{msg}" } 
            end
        end
    
    
    end

end
