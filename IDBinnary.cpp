#include "IDBinnary.h"
#include "math.h"

#include <core.h>
extern Core core;

IDBin::IDBin(ProtoBinIn *proto, QObject *parent) : QObject(parent) {
    setProto(proto);
}

IDBin::~IDBin() {

}

void IDBin::setProto(ProtoBinIn *proto) {
    m_proto = proto;
}

Resp  IDBin::parse() {
    Resp resp_parse = respNone;

    if(m_proto->type() == CONTENT || m_proto->type() == SETTING || m_proto->type() == GETTING) {
        if(m_proto->resp()) {
           m_lastResp = (Resp)m_proto->read<U1>();
           resp_parse = respOk;
        } else {
            m_lastResp = respNone;
            resp_parse = parsePayload(*m_proto);
        }

        if(resp_parse == respOk) {
            m_lastType = m_proto->type();
            m_lastVersion = m_proto->ver();
//            if(m_proto->id() == ID_DIST_SETUP && m_lastResp != respOk) {
//                qInfo("Packet OK: id %u, type %u, ver %u, len %u, resp %u", m_proto->id(), m_proto->type(), m_proto->ver(), m_proto->len(), m_lastResp);
//            }

            emit updateContent(m_lastType, m_lastVersion, m_lastResp);
        } else {
//            if(m_proto->id() != 3) {
//            qInfo("Packet error: id %u, type %u, ver %u, len %u, resp. parse %u", m_proto->id(), m_proto->type(), m_proto->ver(), m_proto->len(), resp_parse);
//            }

        }
    }

    return resp_parse;
}

void IDBin::simpleRequest(Version ver) {
    ProtoBinOut req_out;
    req_out.create(GETTING, ver, id(), m_address);
    requestSpecific(req_out);
    req_out.end();

    sendDataProcessing(req_out);
}

void IDBin::appendKey(ProtoBinOut &proto_out) {
    proto_out.write<U4>(m_key);
}

void IDBin::sendDataProcessing(ProtoBinOut &proto_out) {
    QByteArray data((char*)proto_out.frame(), proto_out.frameLen());
    dataSend(data);
    core.consoleProto(proto_out);
}


Resp IDBinTimestamp::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v0) {
        m_timestamp = proto.read<U4>();
    } else {
        return respErrorVersion;
    }

    return respOk;
}


Resp IDBinDist::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v0) {
        m_dist_mm = proto.read<U4>();
    } else {
        return respErrorVersion;
    }

    return respOk;
}


Resp IDBinChart::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v0) {
        U2 m_seqOffset = proto.read<U2>();
        U2 sampleResol = proto.read<U2>();
        U2 absOffset = proto.read<U2>();

        if(m_seqOffset == 0 && m_chartSizeIncr != 0) {
            for(uint16_t i = 0; i < m_chartSizeIncr; i++) {
                m_completeChart[i] = m_fillChart[i];
            }
            m_chartSize = m_chartSizeIncr;
            m_isCompleteChart = true;
        }

        if(m_seqOffset == 0 || m_sampleResol != sampleResol || m_absOffset != absOffset) {
            m_sampleResol = sampleResol;
            m_absOffset = absOffset;

            m_chartSizeIncr = 0;
        }

        if(m_chartSizeIncr == m_seqOffset) {
            uint16_t part_len = proto.readAvailable();

            if(m_seqOffset + part_len < sizeof (m_fillChart)) {
                proto.read(&m_fillChart[m_chartSizeIncr], part_len);
                m_chartSizeIncr += part_len;
            }
        } else {
            return respErrorPayload;
        }
    } else {
        return respErrorVersion;
    }

    return respOk;
}


Resp IDBinAttitude::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v0) {
        const float scale_to_deg = 0.01f;
        m_yaw = static_cast<float>(proto.read<S2>())*scale_to_deg;
        m_pitch = static_cast<float>(proto.read<S2>())*scale_to_deg;
        m_roll = static_cast<float>(proto.read<S2>())*scale_to_deg;
    } else if(proto.ver() == v1) {
       m_w0 = proto.read<F4>();
       m_w0 = proto.read<F4>();
       m_w0 = proto.read<F4>();
       m_w0 = proto.read<F4>();
    } else {
        return respErrorVersion;
    }

    return respOk;
}

float IDBinAttitude::yaw(Version src_ver) {
    return m_yaw;
}

float IDBinAttitude::pitch(Version src_ver) {
    return m_pitch;
}

float IDBinAttitude::roll(Version src_ver) {
    return m_roll;
}

float IDBinAttitude::w0(Version src_ver) {
    return m_w0;
}
float IDBinAttitude::w1(Version src_ver) {
    return m_w1;
}
float IDBinAttitude::w2(Version src_ver) {
    return m_w2;
}
float IDBinAttitude::w3(Version src_ver) {
    return m_w3;
}

Resp IDBinTemp::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v0) {
        const float scale_to_cels = 0.01f;
        m_temp = static_cast<float>(proto.read<S2>())*scale_to_cels;
    } else {
        return respErrorVersion;
    }

    return respOk;
}


Resp IDBinNav::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v0) {

    } else {
        return respErrorVersion;
    }

    return respOk;
}


Resp IDBinDataset::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v0) {
        uint8_t ch_id = proto.read<U1>();
        if(ch_id < _countof(m_channel)) {
            m_channel[ch_id].id = ch_id;
            m_channel[ch_id].period = proto.read<U4>();
            m_channel[ch_id].mask = proto.read<U4>();
        }
    } else {
        return respErrorVersion;
    }

    return respOk;
}

void IDBinDataset::setChannel(uint8_t ch_id, uint32_t period, uint32_t mask) {
    sendChannel(ch_id, period, mask);
}

uint32_t IDBinDataset::mask(U1 ch_id) {
    uint32_t mask = 0;
    if(ch_id == 0) {
        for(int i = 1; i < 3; i++) {
            mask |= m_channel[i].mask;
        }
    } else if(ch_id < _countof(m_channel)) {
        mask = m_channel[ch_id].mask;
    }
    return mask;
}

void IDBinDataset::setMask(U1 ch_id, uint32_t mask) {
    m_channel[ch_id].id = ch_id;
    m_channel[ch_id].mask = mask;
}

uint32_t IDBinDataset::period(U1 ch_id) {
    if(ch_id < _countof(m_channel)) {
        return m_channel[ch_id].period;
    }
    return 0;
}

void IDBinDataset::setPeriod(U1 ch_id, uint32_t period) {
    sendChannel(ch_id, period, mask(ch_id));
}

void IDBinDataset::sendChannel(U1 ch_id, uint32_t period, uint32_t mask) {
    if(ch_id < _countof(m_channel)) {
        qInfo("ch_id %u, mask %u", ch_id, mask);

        m_channel[ch_id].period = period;

        ProtoBinOut id_out;
        id_out.create(SETTING, v0, id(), m_address);
        id_out.write<U1>(ch_id);
        id_out.write<U4>(period);
        id_out.write<U4>(mask);
        id_out.end();
        sendDataProcessing(id_out);
    }
}


Resp IDBinDistSetup::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v1) {
        m_startOffset = proto.read<U4>();
        m_maxDist = proto.read<U4>();
    } else if(proto.ver() == v2) {
        m_confidence = proto.read<U1>();
    } else {
        return respErrorVersion;
    }

    return respOk;
}

void IDBinDistSetup::setRange(uint32_t start_offset, uint32_t max_dist) {
    m_startOffset = start_offset;
    m_maxDist = max_dist;

    ProtoBinOut id_out;
    id_out.create(SETTING, v1, id(), m_address);
    id_out.write<U4>(start_offset);
    id_out.write<U4>(max_dist);
    id_out.end();
    sendDataProcessing(id_out);
}

void IDBinDistSetup::setConfidence(int confidence) {
    m_confidence = confidence;

    ProtoBinOut id_out;
    id_out.create(SETTING, v2, id(), m_address);
    id_out.write<U1>(m_confidence);
    id_out.end();
    sendDataProcessing(id_out);
}


Resp IDBinChartSetup::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v0) {
        m_sanpleCount = proto.read<U2>();
        m_sanpleResolution = proto.read<U2>();
        m_sanpleOffset = proto.read<U2>();
    } else {
        return respErrorVersion;
    }

    return respOk;
}

void IDBinChartSetup::setV0(uint16_t count, uint16_t resolution, uint16_t offset) {
    m_sanpleCount = count;
    if(count*resolution > 50000) {
        resolution = (50000/count/10)*10;
    }
    m_sanpleResolution = resolution;
    m_sanpleOffset = offset;

    ProtoBinOut id_out;
    id_out.create(SETTING, v0, id(), m_address);
    id_out.write<U2>(count);
    id_out.write<U2>(resolution);
    id_out.write<U2>(offset);
    id_out.end();
    sendDataProcessing(id_out);
}


Resp IDBinTransc::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v0) {
        m_freq = proto.read<U2>();
        m_pulse = proto.read<U1>();
        m_boost = proto.read<U1>();
    } else {
        return respErrorVersion;
    }

    return respOk;
}

void IDBinTransc::setTransc(U2 freq, U1 pulse, U1 boost) {
    m_freq = freq;
    m_pulse = pulse;
    m_boost = boost;

    ProtoBinOut id_out;
    id_out.create(SETTING, v0, id(), m_address);
    id_out.write<U2>(freq);
    id_out.write<U1>(pulse);
    id_out.write<U1>(boost);
    id_out.end();
    sendDataProcessing(id_out);
}


Resp IDBinSoundSpeed::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v0) {
        m_soundSpeed = proto.read<U4>();
    } else {
        return respErrorVersion;
    }

    return respOk;
}

void IDBinSoundSpeed::setSoundSpeed(U4 snd_spd) {
    m_soundSpeed = snd_spd;

    ProtoBinOut id_out;
    id_out.create(SETTING, v0, id(), m_address);
    id_out.write<U4>(snd_spd);
    id_out.end();
    sendDataProcessing(id_out);
}

Resp IDBinUART::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v0) {
        if(checkKeyConfirm(proto.read<U4>())) {
            uint8_t uart_id = proto.read<U1>();

            if(uart_id < _countof(m_uart)) {
                m_uart[uart_id].id = uart_id;
                m_uart[uart_id].baudrate = proto.read<U4>();
            }
        } else {
            return respErrorKey;
        }
    } else if(proto.ver() == v1) {
        if(checkKeyConfirm(proto.read<U4>())) {
            uint8_t uart_id = proto.read<U1>();

            if(uart_id < _countof(m_uart)) {
                m_uart[uart_id].id = uart_id;
                m_uart[uart_id].dev_address = proto.read<U1>();
            }
        } else {
            return respErrorKey;
        }
    } else if(proto.ver() == v2) {
        if(checkKeyConfirm(proto.read<U4>())) {
            devDef_address = proto.read<U1>();
        } else {
            return respErrorKey;
        }
    } else {
        return respErrorVersion;
    }

    return respOk;
}

void IDBinUART::setBaudrate(U4 baudrate) {
    ProtoBinOut id_out;
    id_out.create(SETTING, v0, id(), m_address);
    appendKey(id_out);

    id_out.write<U1>(1);
    m_uart[1].baudrate = baudrate;
    id_out.write<U4>(m_uart[1].baudrate);

    id_out.end();
    sendDataProcessing(id_out);
}

void IDBinUART::setDevAddress(U1 addr) {
    ProtoBinOut id_out;
    id_out.create(SETTING, v1, id(), m_address);
    appendKey(id_out);

    id_out.write<U1>(1);
    m_uart[1].dev_address = addr;
    id_out.write<U1>(addr);

    id_out.end();
    sendDataProcessing(id_out);
}

void IDBinUART::setDevDefAddress(U1 addr) {
    ProtoBinOut id_out;
    id_out.create(SETTING, v2, id(), m_address);
    appendKey(id_out);

    devDef_address = addr;
    id_out.write<U1>(addr);

    id_out.end();
    sendDataProcessing(id_out);
}

Resp IDBinVersion::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v0) {
        m_boardVersionMinor = proto.read<U1>();
        m_boardVersion = (BoardVersion)proto.read<U1>();
        proto.read<U2>();
        proto.read<U2>();
        proto.read<U2>();
        proto.read<U4>();
        proto.read<U2>();
        m_serialNumber = proto.read<U4>();
    } else {
        return respErrorVersion;
    }

    return respOk;
}

Resp IDBinMark::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v0) {
        m_mark = proto.read<U1>();
    } else {
        return respErrorVersion;
    }

    return respOk;
}

void IDBinMark::setMark() {
    ProtoBinOut id_out;
    id_out.create(SETTING, v0, id(), m_address);
    appendKey(id_out);
    id_out.end();

    sendDataProcessing(id_out);
}


Resp IDBinFlash::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v0) {
        if(checkKeyConfirm(proto.read<U4>())) {
        } else {
            return respErrorKey;
        }
    } else if(proto.ver() == v1) {
        if(checkKeyConfirm(proto.read<U4>())) {
        } else {
            return respErrorKey;
        }
    } else if(proto.ver() == v2) {
        if(checkKeyConfirm(proto.read<U4>())) {
        } else {
            return respErrorKey;
        }
    } else {
        return respErrorVersion;
    }

    return respOk;
}

void IDBinFlash::flashing() {
    ProtoBinOut id_out;
    id_out.create(SETTING, v0, id(), m_address);
    appendKey(id_out);
    id_out.end();
    sendDataProcessing(id_out);
}


void IDBinFlash::restore() {
    ProtoBinOut id_out;
    id_out.create(SETTING, v1, id(), m_address);
    appendKey(id_out);
    id_out.end();
    sendDataProcessing(id_out);
}

void IDBinFlash::erase() {
    ProtoBinOut id_out;
    id_out.create(SETTING, v2, id(), m_address);
    appendKey(id_out);
    id_out.end();
    sendDataProcessing(id_out);
}


Resp IDBinBoot::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v0) {
        if(checkKeyConfirm(proto.read<U4>())) {
        } else {
            return respErrorKey;
        }
    } else if(proto.ver() == v1) {
        if(checkKeyConfirm(proto.read<U4>())) {
        } else {
            return respErrorKey;
        }
    } else {
        return respErrorVersion;
    }

    return respOk;
}

void IDBinBoot::reboot() {
    ProtoBinOut id_out;
    id_out.create(SETTING, v0, id(), m_address);
    appendKey(id_out);
    id_out.end();
    sendDataProcessing(id_out);
}

void IDBinBoot::runFW() {
    ProtoBinOut id_out;
    id_out.create(SETTING, v1, id(), m_address);
    appendKey(id_out);
    id_out.end();
    sendDataProcessing(id_out);
}

Resp IDBinUpdate::parsePayload(ProtoBinIn &proto) {
    if(proto.ver() == v0) {
    } else {
        return respErrorVersion;
    }

    return respOk;
}

void IDBinUpdate::setUpdate(QByteArray fw) {
    _fw = fw;
    _fw_offset = 0;
    _nbr_packet = 1;
}

bool IDBinUpdate::putUpdate() {
    uint16_t len_part = 64;
    if(len_part > availSend()) {
        len_part = (uint16_t)availSend();
    }

    if(len_part == 0) {
        return false;
    }

    ProtoBinOut id_out;
    id_out.create(SETTING, v0, id(), m_address);
    id_out.write<U2>(_nbr_packet);

    for(uint16_t i = 0; i < len_part; i++) {
        id_out.write<U1>((U1)_fw[i + _fw_offset]);
    }
    _fw_offset += len_part;

    _nbr_packet++;

    id_out.end();
    sendDataProcessing(id_out);

    return true;
}
