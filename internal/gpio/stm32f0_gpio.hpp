#pragma once
#include "stm32f0xx.h"
#include <cstddef>

enum class Port {A =0,B, C, D, E, F};

class gpio{
public:   
    enum class Mode : uint8_t {
        In          = 0,
        Out         = 1,
        Alt         = 2,
        Analog      = 3,
    };
    enum class Speed : uint8_t {
        Low         = 0,
        Med         = 2,
        High        = 3,
    };
    enum class Pull : uint8_t {
        None        = 0,
        Up          = 1,
        Down        = 2,
    };
    enum class Out : uint8_t {
        PushPull    = 0,
        OpenDrain   = 1,
    };
    enum class AlFunc {
        AF0 = 0b0000,
        AF1 = 0b0001,
        AF2 = 0b0010,
        AF3 = 0b0011,
        AF4 = 0b0100,
        AF5 = 0b0101,
        AF6 = 0b0110,
        AF7 = 0b0111,
    };
    gpio(Port port, size_t pin):m_port(port), m_pin(pin){
        clocken();
    }
    
    void config(Mode moder, Pull pull = Pull::None, Out out = Out::PushPull, Speed speed = Speed::High){
        const uint32_t OTYPER_MASK   = 1UL << m_pin;
        const uint32_t SHIFT         = 2 * m_pin;
        const uint32_t MASK          = 3UL << SHIFT;
       
        m_base->MODER &= ~MASK;
        m_base->PUPDR &= ~MASK;
        m_base->OSPEEDR &= ~MASK;
        
        m_base->MODER |= static_cast<uint8_t>(moder) << SHIFT;
        m_base->PUPDR |= static_cast<uint32_t>(pull) << SHIFT;
        ( out == Out::OpenDrain ) ? ( m_base->OTYPER |= OTYPER_MASK ) : ( m_base->OTYPER  &= ~OTYPER_MASK );
        m_base->OSPEEDR |= static_cast<uint8_t>(speed) << SHIFT;
    }
    void al_config(AlFunc func){
        const uint32_t AFR_SHIFT     = ( m_pin % 8 ) * 4;
        const uint32_t AFR_MASK      = 0x0fUL << AFR_SHIFT;
        
        config(Mode::Alt);        
        auto &afr = m_base->AFR[m_pin / 8];
        afr &= ~AFR_MASK;
        afr |= static_cast<uint32_t>(func) << AFR_SHIFT;
    }
    void set(const bool value){
        const uint32_t SET_MASK      = 1UL << m_pin;
        const uint32_t CLEAR_MASK    = 1UL << ( m_pin + 16 );
        m_base->BSRR = value ? SET_MASK : CLEAR_MASK;
    }  
    bool get() {
        const uint32_t GET_MASK      = 1UL << m_pin;
        return m_base->IDR & GET_MASK;
    }
    size_t pin(){
        return m_pin;
    }
    Port port(){
        return m_port;
    }
private:
    Port m_port;
    size_t m_pin;
    GPIO_TypeDef * m_base;
  
    void clocken(){
        switch (m_port){
        case Port::A:
            RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
            m_base = GPIOA;
            break;
        case Port::B:
            RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
            m_base = GPIOB;
            break;
        case Port::C:
            RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
            m_base = GPIOC;
            break;
        case Port::D:
            RCC->AHBENR |= RCC_AHBENR_GPIODEN;
            m_base = GPIOD;
            break;
        case Port::E:
            RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
            m_base = GPIOE;
            break;
        case Port::F:
            RCC->AHBENR |= RCC_AHBENR_GPIOFEN;
            m_base = GPIOF;
            break;
        };        
    }
};