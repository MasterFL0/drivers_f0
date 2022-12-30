#pragma once
#include "stm32f0xx.h"
#include "globalConfig.hpp"
#include "drivers/gpio/stm32f0_gpio.hpp"
#include <cstddef>
typedef void ( *UserIrqHandler_t )();

class IrqHelper {
public:
    static void exti0_1IrqHandler() {
        EXTI->PR = EXTI->PR & ( 1UL << 0 | 1UL << 1 );  // Маска для exti0_1IrqHandler
        HandlerExecutor<0, 2>();
    }
    static void exti2_3IrqHandler() {
        EXTI->PR = EXTI->PR & ( 1UL << 2 | 1UL << 3 ); // Маска для exti2_3IrqHandler
        HandlerExecutor<2, 2>();
    }
    static void exti4_15IrqHandler() {
        static const auto MASK_EXTI4_15 = 0xFFF8;
        EXTI->PR = EXTI->PR & MASK_EXTI4_15;
        HandlerExecutor<4, 12>();
    }
    static UserIrqHandler_t m_userHandler[];
private:
    static const auto USER_HANDLER_SIZE = 16;
 
    template<size_t Start, size_t Count>
        static void HandlerExecutor() {
            for ( auto idx = 0; idx < Start + Count; idx++ )
                if (m_userHandler[idx]) m_userHandler[idx]();
        }
};

class exti{
public:
    enum class Front : uint8_t{
        Rising,
        Falling,
        All,
    };
    exti(gpio gpio):m_gpio(gpio) {}
    void config(Front front = Front::Rising){
        EXTI->IMR |= 1UL << m_gpio.pin();
        switch (front) {
        case Front::Rising:
           EXTI->RTSR |= 1UL << m_gpio.pin();
            break;
        case Front::Falling:
           EXTI->FTSR |= 1UL << m_gpio.pin();
            break;
        case Front::All:
            EXTI->RTSR |= 1UL << m_gpio.pin();
            EXTI->FTSR |= 1UL << m_gpio.pin();
            break;
        };
        RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;
        SYSCFG->EXTICR[ m_gpio.pin()/4] |=  (static_cast<uint8_t>(m_gpio.port())) << (((m_gpio.pin())%4)*4);
        const auto idx = m_gpio.pin() < 2 ? 0 : m_gpio.pin() < 4 ? 1 : 2;
        NVIC_SetPriority(vectorName[idx], priority[idx]);
        NVIC_ClearPendingIRQ(vectorName[idx]);
        NVIC_EnableIRQ(vectorName[idx]);
    }
    void setIrqHandler(UserIrqHandler_t _handler){
        IrqHelper::m_userHandler[m_gpio.pin()] = _handler;
    }
    
private:
    gpio  m_gpio;
    const IRQn_Type vectorName[3] = {
        EXTI0_1_IRQn, EXTI2_3_IRQn, EXTI4_15_IRQn,
    };
    const uint32_t priority[3] = {
        EXTI0_1_IRQ_PRIORITY, EXTI2_3_IRQ_PRIORITY, EXTI4_15_IRQ_PRIORITY
    };
};


