/*
 * MathOpPort.hpp
 *
 *  Created on: Monday, 31 December 2018
 *  Author:     rdaruwala
 *
 */
#ifndef MATHOPPORT_HPP_
#define MATHOPPORT_HPP_

#include <cstring>
#include <cstdio>
#include <Fw/Cfg/Config.hpp>
#include <Fw/Port/InputPortBase.hpp>
#include <Fw/Port/OutputPortBase.hpp>
#include <Fw/Comp/PassiveComponentBase.hpp>
#include <Fw/Types/BasicTypes.hpp>
#include <Fw/Types/Serializable.hpp>
#include <Fw/Types/StringType.hpp>


namespace Ref {

    typedef enum {
        MATH_ADD,
        MATH_SUB,
        MATH_MULTIPLY,
        MATH_DIVIDE,
        MathOperation_MAX
    } MathOperation; //!< MathOperation enumeration argument

    /// Input MathOp port description
    /// 

    class InputMathOpPort : public Fw::InputPortBase  {
    public:
        enum {
            SERIALIZED_SIZE = sizeof(F32) + sizeof(F32) + sizeof(NATIVE_INT_TYPE) //!< serialized size of port arguments
        };
        typedef void (*CompFuncPtr)(Fw::PassiveComponentBase* callComp, NATIVE_INT_TYPE portNum, F32 val1, F32 val2, MathOperation operation); //!< port callback definition

        InputMathOpPort(void); //!< constructor
        void init(void); //!< initialization function
        void addCallComp(Fw::PassiveComponentBase* callComp, CompFuncPtr funcPtr); //!< call to register a component
        void invoke(F32 val1, F32 val2, MathOperation operation); //!< invoke port interface
    protected:
    private:
        CompFuncPtr m_func; //!< pointer to port callback function
#if FW_PORT_SERIALIZATION == 1        
        void invokeSerial(Fw::SerializeBufferBase &buffer); //!< invoke the port with serialized arguments
#endif
};
    /// Input MathOp port description
    /// 
    
    class OutputMathOpPort : public Fw::OutputPortBase {
      public: 
        OutputMathOpPort(void);
        void init(void);
        void addCallPort(InputMathOpPort* callPort);
        void invoke(F32 val1, F32 val2, MathOperation operation);
      protected:
      private:
        InputMathOpPort* m_port;
    };
} // end namespace Ref
#endif /* MATHOP_HPP_ */

