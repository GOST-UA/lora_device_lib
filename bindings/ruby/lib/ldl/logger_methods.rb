require 'logger'

module LDL

    LOG_FORMATTER = Proc.new do |severity, datetime, progname, msg|
        "#{severity.ljust(5)} [#{datetime.strftime("%Y-%m-%d %H:%M:%S")}] #{msg}\n"
    end

    module LoggerMethods
    
        def log_info(msg)
            Logger.info { "#{SystemTime.time.to_s.rjust(10)} #{msg}" } 
        end
        
        def log_error(msg)
            Logger.error { "[#{SystemTime.time.to_s.rjust(10)}] #{msg}" } 
        end
        
        def log_debug(msg)
            Logger.debug { "[#{SystemTime.time.to_s.rjust(10)}] #{msg}" } 
        end
    
    end

end
