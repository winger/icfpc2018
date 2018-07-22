#include "command.h"

void Command::Encode(vector<uint8_t>& v) const
{
    if (type == Halt)
        v.push_back(0b11111111);
    else if (type == Wait)
        v.push_back(0b11111110);
    else if (type == Flip)
        v.push_back(0b11111101);
    else if (type == SMove)
    {
        uint16_t t = EncodeLCD(cd1);
        v.push_back(((t >> 8) << 4) | 0b00000100);
        v.push_back(t);
    }
    else if (type == LMove)
    {
        uint16_t t1 = EncodeSCD(cd1);
        uint16_t t2 = EncodeSCD(cd2);
        v.push_back(((t1 >> 8) << 4) | ((t2 >> 8) << 6) | 0b00001100);
        v.push_back(t1 | (t2 << 4));
    }
    else if (type == FusionP)
    {
        uint8_t t = EncodeNCD(cd1);
        v.push_back((t << 3) | 0b00000111);
    }
    else if (type == FusionS)
    {
        uint8_t t = EncodeNCD(cd1);
        v.push_back((t << 3) | 0b00000110);
    }
    else if (type == Fission)
    {
        uint8_t t = EncodeNCD(cd1);
        v.push_back((t << 3) | 0b00000101);
        v.push_back(m);
    }
    else if (type == Fill)
    {
        uint8_t t = EncodeNCD(cd1);
        v.push_back((t << 3) | 0b00000011);
    }
    else if (type == Void)
    {
        uint8_t t = EncodeNCD(cd1);
        v.push_back((t << 3) | 0b00000010);
    }
    else if (type == GFill)
    {
        uint8_t t1 = EncodeNCD(cd1);
        uint32_t t2 = EncodeFCD(cd2);
        v.push_back((t1 << 3) | 0b00000001);
        v.push_back(t2 >> 16);
        v.push_back(t2 >> 8);
        v.push_back(t2);
    }
    else if (type == GVoid)
    {
        uint8_t t1 = EncodeNCD(cd1);
        uint32_t t2 = EncodeFCD(cd2);
        v.push_back((t1 << 3) | 0b00000000);
        v.push_back(t2 >> 16);
        v.push_back(t2 >> 8);
        v.push_back(t2);
    }
    else
        assert(false);
}

void Command::Decode(const vector<uint8_t>& v, size_t& pos)
{
    uint8_t u1 = v[pos++];
    if (u1 == 0b11111111)
        type = Halt;
    else if (u1 == 0b11111110)
        type = Wait;
    else if (u1 == 0b11111101)
        type = Flip;
    else if ((u1 & 0b00001111) == 0b00000100)
    {
        type = SMove;
        uint16_t u2 = v[pos++];
        uint16_t t = ((uint16_t(u1) >> 4) << 8) | u2;
        cd1 = DecodeLCD(t);
    }
    else if ((u1 & 0b00001111) == 0b00001100)
    {
        type = LMove;
        uint16_t u2 = v[pos++];
        uint16_t t1 = (((uint16_t(u1) >> 4) & 3) << 8) | (u2 & 15);
        uint16_t t2 = (((uint16_t(u1) >> 6) & 3) << 8) | ((u2 >> 4) & 15);
        cd1 = DecodeSCD(t1);
        cd2 = DecodeSCD(t2);
    }
    else if ((u1 & 0b00000111) == 0b00000111)
    {
        type = FusionP;
        cd1 = DecodeNCD(u1 >> 3);
    }
    else if ((u1 & 0b00000111) == 0b00000110)
    {
        type = FusionS;
        cd1 = DecodeNCD(u1 >> 3);
    }
    else if ((u1 & 0b00000111) == 0b00000101)
    {
        type = Fission;
        cd1 = DecodeNCD(u1 >> 3);
        m = v[pos++];
    }
    else if ((u1 & 0b00000111) == 0b00000011)
    {
        type = Fill;
        cd1 = DecodeNCD(u1 >> 3);
    }
    else if ((u1 & 0b00000111) == 0b00000010)
    {
        type = Void;
        cd1 = DecodeNCD(u1 >> 3);
    }
    else if ((u1 & 0b00000111) == 0b00000001)
    {
        type = GFill;
        uint32_t u2 = v[pos++];
        uint32_t u3 = v[pos++];
        uint32_t u4 = v[pos++];
        cd1 = DecodeNCD(u1 >> 3);
        cd2 = DecodeFCD((u2 << 16) | (u3 << 8) | u4);
    }
    else if ((u1 & 0b00000111) == 0b00000000)
    {
        type = GVoid;
        uint32_t u2 = v[pos++];
        uint32_t u3 = v[pos++];
        uint32_t u4 = v[pos++];
        cd1 = DecodeNCD(u1 >> 3);
        cd2 = DecodeFCD((u2 << 16) | (u3 << 8) | u4);
    }
    else {
        assert(false);
    }
}

ostream& operator<<(ostream& s, const Command& c) {
    s << "{";
    switch (c.type) {
        case Command::Halt:
            s << "Halt";
            break;
        case Command::Wait:
            s << "Wait";
            break;
        case Command::Flip:
            s << "Flip";
            break;
        case Command::SMove:
            s << "SMove, cd1=" << c.cd1;
            break;
        case Command::LMove:
            s << "LMove, cd1=" << c.cd1 << ", cd2=" << c.cd2;
            break;
        case Command::FusionP:
            s << "FusionP";
            break;
        case Command::FusionS:
            s << "FusionS";
            break;
        case Command::Fission:
            s << "Fission";
            break;
        case Command::Fill:
            s << "Fill, cd1=" << c.cd1;
            break;
        case Command::Void:
            s << "Void, cd1=" << c.cd1;
            break;
        case Command::GFill:
            s << "GFill";
            break;
        case Command::GVoid:
            s << "GVoid";
            break;
    }
    s << "}";
    return s;
}
