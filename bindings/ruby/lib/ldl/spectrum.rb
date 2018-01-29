module LDL

    class Spectrum
    
        attr_reader :devices
    
        def initialize(broker)
            
            @broker = broker
            
            @devices = {}
            
            broker.subscribe 'up' do |msg|       
                devices[msg[:eui]] = {:eui => msg[:eui]}
            end

            broker.subscribe 'down' do |msg|
                devices.delete msg[:eui]
            end

            broker.subscribe 'send' do |msg|
                devices.values.each do |d|                
                    if d[:eui] != msg[:eui]
                        broker.publish msg, msg[:eui].to_s                        
                    end
                end
            end
               
        end
    end

end
