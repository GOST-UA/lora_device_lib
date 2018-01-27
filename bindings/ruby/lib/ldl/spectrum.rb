module LDL

    class Spectrum
        def initialize(broker)
            
            @broker = broker
            @gateway = []
            @device = []            

            broker.subscribe "gateway_up", "gateway_down", "device_up", "device_down" do |msg, topic|

                case topic
                when "gateway_up"                    
                when "device_up"
                when "gateway_down"
                when "device_down"
                end

            end

            broker.subscribe "tx" do |msg|

                

            end

        end
    end

end
