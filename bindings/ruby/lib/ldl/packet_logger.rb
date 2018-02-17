require 'base64'

module LDL

    # logs 'tx_begin' topics
    class PacketLogger
        
        attr_reader :broker

        def initialize(broker, *output)
            
            raise TypeError unless output.detect{|o| not (o.respond_to? :write and o.respond_to? :flush)}.nil?
            
            @output = output
            
            log "system_time, eui, message_type, frequency, spreading_factor, bandwidth, power, air_time, size, message\r\n"
            
            broker.subscribe('tx_begin') do |msg|
                                    
                begin
                    type = Frame.decode(msg[:data]).class.name.split("::").last
                rescue                    
                    type = 'unknown'
                end
                                    
                log "#{ticks_to_s(msg[:time]).round(5)}, #{msg[:eui]}, #{type}, #{msg[:freq]}, #{msg[:sf]}, #{msg[:bw]/1000}, #{msg[:power]}, #{ticks_to_s(msg[:airTime])}, #{msg[:data].size}, #{Base64.strict_encode64(msg[:data])}\r\n"        
        
            end
            
        end
        
        def log(msg)
            @output.each{ |o| o.write(msg); o.flush }            
        end
        
        def ticks_to_s(ticks)
            ticks / MAC::TICKS_PER_SECOND.to_f
        end
        
        private :ticks_to_s, :log
        
    end

end
