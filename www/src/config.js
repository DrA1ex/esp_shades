import {AppConfigBase} from "./lib/index.js";

import {PropertyConfig} from "./props.js";
import {PacketType} from "./cmd.js";


export class Config extends AppConfigBase {
    nightMode;
    stepperCalibration;
    stepperConfig;
    sysConfig;

    status;

    constructor() {
        super(PropertyConfig);

        this.lists["wifiMode"] = [
            {code: 0, name: "AP"},
            {code: 1, name: "STA"},
        ];
    }

    get cmd() {return PacketType.GET_CONFIG;}

    async load(ws) {
        const statePacket = await ws.request(PacketType.GET_STATE)
        this.status = this.#parseState(statePacket.parser());

        await super.load(ws);
    }

    parse(parser) {
        this.stepperCalibration = {
            offset: parser.readUint16(),
            openPosition: parser.readInt32()
        };

        this.nightMode = {
            enabled: parser.readBoolean(),
            startTime: parser.readUint32(),
            endTime: parser.readUint32()
        };

        this.stepperConfig = {
            reverse: parser.readBoolean(),
            resolution: parser.readUint16(),
            openSpeed: parser.readUint16(),
            closeSpeed: parser.readUint16(),
            acceleration: parser.readUint16(),
            homingSpeed: parser.readUint16(),
            homingSpeedSecond: parser.readUint16(),
            homingSteps: parser.readInt32(),
            homingStepsMax: parser.readInt32()
        };

        this.sysConfig = {
            mdnsName: parser.readFixedString(32),

            wifiMode: parser.readUint8(),
            wifiSsid: parser.readFixedString(32),
            wifiPassword: parser.readFixedString(32),

            wifiConnectionCheckInterval: parser.readUint32(),
            wifiMaxConnectionAttemptInterval: parser.readUint32(),

            stepperPin1: parser.readUint8(),
            stepperPin2: parser.readUint8(),
            stepperPin3: parser.readUint8(),
            stepperPin4: parser.readUint8(),
            stepperPinEn: parser.readUint8(),

            endstopPin: parser.readUint8(),
            endstopHighState: parser.readBoolean(),

            timeZone: parser.readFloat32(),

            mqtt: parser.readBoolean(),
            mqttHost: parser.readFixedString(32),
            mqttPort: parser.readUint16(),
            mqttUser: parser.readFixedString(32),
            mqttPassword: parser.readFixedString(32)
        };
    }

    #parseState(parser) {
        return {
            homed: parser.readBoolean(),
            moving: parser.readBoolean(),
            position: parser.readInt32(),
            position_target: parser.readFloat32(),
            offset: parser.readInt16(),
        }
    }
}