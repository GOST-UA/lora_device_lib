module LDL

    class Radio
    
        attr_accessor :broker, :mac, :active
        attr_reader :buffer
    
        def initialize(mac, broker)

            raise "SystemTime must be defined" unless defined? SystemTime
            
            @broker = broker 
            @mac = mac
            @buffer = Queue.new
            
            @active = []
            
            broker.subscribe "tx_begin" do |m1|            
                active << m1 unless m1[:eui] == mac.devEUI
            end
            
            broker.subscribe "tx_end" do |m2|            
                active.delete_if{|m1|m1[:eui] == m2[:eui]}      
            end
            
        end
    
        def resetHardware
            # this is no good since we are inside the event loop
            #SystemTime.wait(1)        
        end
    
        def transmit(data, **settings)            
            
            bw = settings[:bw]
            sf = settings[:sf]
            freq = settings[:freq]
            
            msg = {
                :eui => mac.devEUI, 
                :time => SystemTime.time,
                :data => data.dup,
                :sf => sf,
                :bw => bw,
                :cr => settings[:cr],
                :freq => freq,
                :power => settings[:power],                    
                :channel => settings[:channel]
            }
            
            broker.publish msg, "tx_begin"
            
            SystemTime.onTimeout(mac.class.transmitTimeUp(bw, sf, data.size)) do
            
                mac.io_event :tx_complete, SystemTime.time
                                
                broker.publish({:eui => mac.devEUI}, "tx_end")
                
            end           
            
            true
        
        end
            
        def receive(**settings)
            
            bw = settings[:bw]
            sf = settings[:sf]
            freq = settings[:freq]
            
            tx_begin = nil
            
            t_sym = (2 ** settings[:sf]) / settings[:bw].to_f
            
            window = Range.new(SystemTime.time, SystemTime.time + ((settings[:timeout] * t_sym) * 100000).ceil.to_i)
            
            puts window
            
            # work out if there were any overlapping transmissions at window timeout
            SystemTime.onTimeout( settings[:timeout] * t_sym ) do
               
                active.detect do |m1|
               
                    m1[:sf] == sf and m1[:bw] == bw and m1[:freq] == freq and window.include? m1[:time] 
                    
                    # todo calculate if sufficient symbols fell within window
                    
                end.tap do |m1|
                
                    if m1
                
                        puts "found packet at end of window"
                
                        tx_end = broker.subscribe "tx_end" do |m2|
                        
                            if m2[:eui] == m1[:eui]
                            
                                broker.unsubscribe tx_end
                                buffer.push(m1[:data].dup)                                
                                mac.io_event :rx_ready, SystemTime.time      
                                
                            end
                        
                        end 
                    
                    else
                                  
                        mac.io_event :rx_timeout, SystemTime.time 
                    
                    end
                    
                end
               
            end
            
            true
                    
        end
        
        def collect  
            buffer.pop              
        end
        
        def sleep
        end
        
    end


end
