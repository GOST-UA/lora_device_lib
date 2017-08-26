require 'json'

module LDL::Semtech

    class RXPacket
    
=begin
         Name |  Type  | Function
        :----:|:------:|--------------------------------------------------------------
         time | string | UTC time of pkt RX, us precision, ISO 8601 'compact' format
         tmms | number | GPS time of pkt RX, number of milliseconds since 06.Jan.1980
         tmst | number | Internal timestamp of "RX finished" event (32b unsigned)
         freq | number | RX central frequency in MHz (unsigned float, Hz precision)
         chan | number | Concentrator "IF" channel used for RX (unsigned integer)
         rfch | number | Concentrator "RF chain" used for RX (unsigned integer)
         stat | number | CRC status: 1 = OK, -1 = fail, 0 = no CRC
         modu | string | Modulation identifier "LORA" or "FSK"
         datr | string | LoRa datarate identifier (eg. SF12BW500)
         datr | number | FSK datarate (unsigned, in bits per second)
         codr | string | LoRa ECC coding rate identifier
         rssi | number | RSSI in dBm (signed integer, 1 dB precision)
         lsnr | number | Lora SNR ratio in dB (signed float, 0.1 dB precision)
         size | number | RF packet payload size in bytes (unsigned integer)
        data | string | Base64 encoded RF packet payload, padded
=end
    
        def self.from_json
            self.new
        end
    
        def initialize(**param)

            init = Proc.new do |iv_name, klass, default|
                if param[iv_name]
                    raise TypeError unless param[iv_name].kind_of? klass
                    instance_variable_set(iv_name, param[iv_name])
                else
                    instance_variable_set(iv_name, default)
                end
            end
            
            init.run(:time, Time, Time.now) 
            init.run(:freq, Integer, 0) 
            init.run(:chan, Integer, 0) 
            init.run(:rfch, Integer, 0) 
            init.run(:stat, Integer, 1) 
            init.run(:stat, Integer, 1) 
            init.run(:modu, String, "LORA") 
            init.run(:datr, String, "LORA") 
            
        end
        
        def to_json
            {
                :time => @time.iso8601,
                :freq => @freq,
                :chan => @chan,
                :rfch => @rfch,
                :stat => @stat,
                :modu => @modu,
                :datr => @datr,
                :codr => @codr,
                :rssi => @rssi,
                :size => @data.size,
                :data => Base64.new(@data)
            }.to_json
        end
    
    end
    
end
