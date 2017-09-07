require 'json'
require 'time'
require 'base64'

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

        def self.from_json(obj)
            self.new
        end
        
        attr_reader :time
        attr_reader :tmms
        attr_reader :freq
        attr_reader :chan
        attr_reader :rfch
        attr_reader :stat
        attr_reader :modu
        
        attr_reader :bw
        attr_reader :sf
        
        def datr
            if @modu == "FSK"
                @dr
            else
                @datr
            end
        end
        
        attr_reader :codr
        attr_reader :rssi
        attr_reader :lsnr
    
        def size
            @data.size
        end
    
        attr_reader :data
    
        def self.from_json
            begin
                self.new(
                    time: Time.parse(msg["time"]),
                    tmms: msg["tmms"],
                    tmst: msg["tmst"],
                    freq: msg["freq"],
                    chan: msg["chan"],
                    rfch: msg["rfch"],
                    stat: msg["stat"],
                    modu: msg["modu"],
                    datr: msg["datr"],
                    codr: msg["codr"],
                    rssi: msg["rssi"],
                    lsnr: msg["lsnr"],
                    size: msg["size"],
                    data: Base64.decode64(msg["data"])
                )                    
            rescue
                raise ArgumentError
            end            
        end
    
        def initialize(**param)

            init = Proc.new do |iv_name, klass, default, &validation|
                
                if param[iv_name]
                    raise TypeError unless param[iv_name].kind_of? klass                                        
                    value = param[iv_name]                    
                    instance_variable_set("@#{iv_name}", value)
                else
                    instance_variable_set("@#{iv_name}", default)        
                end
                
            end
            
            init.call(:time, Time, Time.now) 
            init.call(:freq, Numeric, 0) 
            init.call(:chan, Integer, 0)
            init.call(:rfch, Integer, 0) 
            init.call(:stat, Symbol, :ok) do |value|
                raise RangeError unless [:ok, :fail, :nocrc].include? value
                value
            end 
            init.call(:modu, String, "LORA") do |value|
                raise RangeError unless ["LORA", "FSK"].include? value
                value            
            end
                        
            if param[:datr]
                if modu == "LORA"
                    if not(match = param[:datr].match(/^SF(?<sf>[0-9])BW(?<bw>[0-9]+)$/))
                        raise ArgumentError
                    end
                    @datr = param[:datr]
                    @sf = match[:sf]
                    @bw = match[:bw]
                else
                    raise ArugmentError unless not(param[:datr].kind_of? Numeric)
                end
            else
                @datr = "SF7BW125"
                @bw = 125000
                @sf = 7
            end
            
            init.call(:codr, String, "4/5") do |value|
                raise ArgumentError unless value[/^[0-9]+\/[0-9]+$/]
                value
            end
            
            init.call(:rssi, Numeric, 0) 
            init.call(:lsnr, Numeric, 0)             
            init.call(:data, String, "") 
            
            if param[:size]            
                raise TypeError unless param[:size].kind_of? Numeric
                raise ArgumentError.new("explicit size does not match size of data") unless param[:size] == data.size
            end
            
        end
        
        def to_json
            {
                :time => @time.iso8601,
                :freq => freq,
                :chan => chan,
                :rfch => rfch,
                :stat => stat,
                :modu => modu,
                :datr => datr,
                :codr => codr,
                :rssi => rssi,
                :lsnr => lsnr,
                :size => size,
                :data => Base64.encode64(@data)
            }.to_json
        end
    
    end
    
end
