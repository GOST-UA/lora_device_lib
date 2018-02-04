module LDL

    class Radio
    
        attr_accessor :buffer, :mode, :broker, :mac
    
        def initialize(mac, broker)

            raise "SystemTime must be defined" unless defined? SystemTime
            
            @buffer = ""     
            @broker = broker 
            @mac = mac
            
        end
    
        def resetHardware
            SystemTime.wait(1)        
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
            }
            
            broker.publish msg, "tx_begin"
            
            SystemTime.onTimeout(mac.class.onAirTime(bw, sf, data.size)) do
            
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
            
            # fixme: core isn't passing a timeout
            # listen for the interval
            #to = SystemTime.onTimeout(settings[:timeout]) do
            to = SystemTime.onTimeout(0.001) do
               
               broker.unsubscribe tx_begin
               
               mac.io_event :rx_timeout, SystemTime.time 
               
            end
            
            # begin listening
            tx_begin = broker.subscribe "tx_begin" do |m1|
            
                if m1[:sf] == sf and m1[:bw] == bw and m1[:freq] == freq
            
                    broker.cancel to
                    broker.unsubscribe tx_begin
                    
                    tx_end = broker.subscribe "tx_end" do |m2|            
                    
                        if m1[:eui] == m2[:eui]
                        
                            broker.unsubscribe tx_end
                            
                            mac.io_event :rx_ready, SystemTime.time
                            
                            @buffer = m1[:data].dup
                        
                        end
                        
                    end
                    
                end
            
            end
            
            true
                    
        end
        
        def collect        
            buffer
        end
        
        def sleep
        end
        
    end


end
