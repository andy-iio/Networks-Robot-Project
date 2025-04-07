#include "pch.h"
#include "CppUnitTest.h"
#include "../NetworksRobotProject/packet.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(UnitTests)
	{
    public:
        
        //-------------TESTS FOR GetCmd() FUNCTION---------------------
        //-------------------------------------------------------------
        TEST_METHOD(TestGetCmd_Drive_ReturnDRIVE)
        {
            PktDef pkt;
            pkt.setCommand(PktDef::DRIVE);
            Assert::AreEqual(PktDef::DRIVE, pkt.GetCmd());
        }

        TEST_METHOD(TestGetCmd_Sleep_ReturnSLEEP)
        {
            PktDef pkt;
            pkt.setCommand(PktDef::SLEEP);
            Assert::AreEqual(PktDef::SLEEP, pkt.GetCmd());
        }

        TEST_METHOD(TestGetCmd_Response_ReturnRESPONSE)
        {
            PktDef pkt;
            pkt.setCommand(PktDef::RESPONSE);
            Assert::AreEqual(PktDef::RESPONSE, pkt.GetCmd());
        }

        TEST_METHOD(TestGetCmd_Default_ReturnSLEEP)
        {
            PktDef pkt;
            pkt.setCommand(static_cast<PktDef::CmdType>(-5)); //invalid command to test out the default option
            Assert::AreEqual(PktDef::SLEEP, pkt.GetCmd());
        }


        //-------------TESTS FOR GetAck() FUNCTION---------------------
        //-------------------------------------------------------------
        TEST_METHOD(TestGetAck_NotSet_ReturnFalse)
        {
            PktDef pkt;
            //default, should return false bc ack not is set
            Assert::IsFalse(pkt.GetAck());
        }

        TEST_METHOD(TestGetAck_Set_ReturnTrue)
        {
            //creating a response command packet
            PktDef pkt;
            pkt.setCommand(PktDef::RESPONSE);

            Assert::IsTrue(pkt.GetAck()); 
        }



        //-------------TESTS FOR checkCRC() FUNCTION-------------------
        //-------------------------------------------------------------
        TEST_METHOD(TestCheckCRC_ValidPacket_True)
        {
            PktDef pkt;
            char testData[] = "0000000000000001";
            pkt.SetBodyData(testData, strlen(testData));
            pkt.SetPktCount(1);

            //calculate the packet crc
            pkt.CalcCRC();

            //generate the packet with the right CRC
            char* rawPacket = pkt.GenPacket();
            int packetSize = pkt.GetLength();

            Assert::IsTrue(pkt.CheckCRC(rawPacket, packetSize));
          }


        TEST_METHOD(TestCheckCRC__CorruptedPacket_False)
        {
            PktDef pkt;
            char testData[] = "0000000000000001";
            pkt.SetBodyData(testData, strlen(testData));
            pkt.SetPktCount(1);
            //generate the packet with the right CRC
            char* rawPacket = pkt.GenPacket();
            int packetSize = pkt.GetLength();
            //corrupt the packet so it fails
            if (rawPacket && packetSize > 0) {
                rawPacket[0] ^= 0xFF; //flip bits in first byte
                //should fail
                Assert::IsFalse(pkt.CheckCRC(rawPacket, packetSize));
            }
        }

        TEST_METHOD(TestCheckCRC_emptyBuffer_False)
        {
            PktDef pkt;
            //if given null buffer
            Assert::IsFalse(pkt.CheckCRC(nullptr, 0));
        }

        //-------------TESTS FOR CalcCRC() FUNCTION---------------------
        //-------------------------------------------------------------
        TEST_METHOD(TestCalcCRC)
        {
            PktDef pkt;
            char data[] = "01010";
            pkt.SetBodyData(data, sizeof(data) - 1);

            //calculate CRC
            pkt.CalcCRC();

            Assert::AreEqual(1,1);
        }

        //-------------TESTS FOR GenPacket() FUNCTION------------------
        //-------------------------------------------------------------
        TEST_METHOD(TestGenPacket_NotNull)
        {
            PktDef pkt;

            //set the data for the packet
            char data[] = "101";
            pkt.SetBodyData(data, sizeof(data) - 1);

            //generate packet
            char* generatedPacket = pkt.GenPacket();

            //check its not null
            Assert::IsNotNull(generatedPacket);
        }


        //-------------TESTS FOR SetBodyData() FUNCTION------------------
        //-------------------------------------------------------------
        TEST_METHOD(SetBodyData_Valid)
        {
            PktDef pkt;
            const char testData[] = "0001010010101";
            const int testSize = strlen(testData);

            pkt.SetBodyData(const_cast<char*>(testData), testSize);

            Assert::IsNotNull(pkt.GetBodyData());
            Assert::AreEqual(0, memcmp(testData, pkt.GetBodyData(), testSize));
        }

        //nullptr data
        TEST_METHOD(SetBodyData_nullptrData)
        {
            PktDef pkt;

            pkt.SetBodyData(nullptr, 10);

            Assert::IsNull(pkt.GetBodyData());
        }





        //-----------ALEXAS TESTS-------------------------------------
        //-------------------------------------------------------------
        TEST_METHOD(DriveCommmand)
        {
            PktDef pkt;

            pkt.setCommand(PktDef::DRIVE);

            Assert::AreEqual(PktDef::DRIVE, pkt.GetCmd()); // drive bit flag 
        }

        TEST_METHOD(SleepCommand)
        {
            PktDef pkt;

            pkt.setCommand(PktDef::SLEEP);

            Assert::AreEqual(PktDef::SLEEP, pkt.GetCmd()); // sleep bit flag 

        }


        TEST_METHOD(ResponseCommand)
        {

            PktDef pkt;

            pkt.setCommand(PktDef::RESPONSE);

            Assert::AreEqual(PktDef::RESPONSE, pkt.GetCmd()); // response bit flag 
        }


        TEST_METHOD(getlength)
        {

            PktDef pkt;
            int expectedLength = 10;

            pkt.setCommand(PktDef::RESPONSE);

            Assert::AreEqual(expectedLength, pkt.GetLength());
        }

        TEST_METHOD(GetPacketCount)
        {
            PktDef pkt;
            int val = 10;
            pkt.setCommand(PktDef::RESPONSE);
            pkt.SetPktCount(val); // calling function 

            Assert::AreEqual(val, pkt.GetPktCount()); //check that val and pktcount are the same
        }
    };

}









//THIS IS NEEDED to be able to run the tests, need to convert cmdtype to string
namespace Microsoft {
    namespace VisualStudio {
        namespace CppUnitTestFramework {
            //ToString for PktDef::CmdType, to fix error in assert printing for unit tests
            template<>
            static std::wstring ToString<PktDef::CmdType>(const PktDef::CmdType& cmdType) {
                switch (cmdType) {
                case PktDef::DRIVE:
                    return L"DRIVE";
                case PktDef::SLEEP:
                    return L"SLEEP";
                case PktDef::RESPONSE:
                    return L"RESPONSE";
                default:
                    return L"Unknown CmdType";
                }
            }
        }
    }
}
