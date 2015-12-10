/* ========================================
 *
 * ITM Test project
 * 
 * TODO:
 *
 * Create a ITM Component
 *
 * ========================================
*/
#include <project.h>

#define MLOGIC_DEBUG_SWV_CLK_EN     0x4
#define MLOGIC_DEBUG_SWV_CLK_SEL    0x8
#define TPIU_PROTOCOL_MANCHESTER    0x1
#define TPIU_PROTOCOL_UART          0x2
#define ETM_CTL_PRT_SEL_Pos         11
#define ETM_CTL_STALL_uP_Pos        7
#define ETM_CTL_BRNCH_OUT_Pos       8
#define ETM_CTL_PROG_Pos            10
#define ETM_TRACE_EN_CTRL1_TRACE_CTRL_EN_Pos    25

void configure_tracing(void);
void ITM_Print(uint32_t port, const char *p);
void ITM_SendValue(uint32_t port, uint32_t value);

int main()
{
    CyGlobalIntEnable;

    configure_tracing();
    
    CyDelay(50);
    
    ITM_Print(0, "Boot");
    
    for(;;){
        CyDelay(50);
        ITM_Print(0, "On"); /* Use ITM Channel 0 for ITM printf style debug */
        CyDelay(50);
        ITM_Print(0, "Off");
    }
    
}

void configure_tracing(void){
    
    // Set following bits in the MLOGIC_DEBUG register:
	// swv_clk_en = 1, swv_clk_sel = 1 (CPUCLK/2).
    // MLOGIC_DEBUG_SWV_CLK_EN  = Serial Wire View clock enable.
    // MLOGIC_DEBUG_SWV_CLK_SEL = Serial Wire Viewer trace clock select, when set to 1 clk_cpu/2 is used for trace clock, set to 0 trace clock is used
    // CY_SET_REG8(CYREG_MLOGIC_DEBUG, (CY_GET_REG8(CYREG_MLOGIC_DEBUG) | MLOGIC_DEBUG_SWV_CLK_EN | MLOGIC_DEBUG_SWV_CLK_SEL));
    CY_SET_REG8(CYREG_MLOGIC_DEBUG, (CY_GET_REG8(CYREG_MLOGIC_DEBUG) | MLOGIC_DEBUG_SWV_CLK_EN | MLOGIC_DEBUG_SWV_CLK_SEL));
    
    // Enable output features
    // Debug Exception and Monitor Control Register
    // CoreDebug_DEMCR_TRCENA_Msk = Trace system enable; to use DWT, ETM, ITM and TPIU, this bit must be set to 1
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    
    //Set Output Divisor Register (13 bits)
    // Async Clock Prescaler Register
    // PRESCALER[12:0] = Divisor for TRACECLKIN is Prescaler + 1
    TPI->ACPR = 0; // Trace clock = HCLK / ( x + 1 ) = 6MHz
    
    //Setup the output encoding style
    // Selected Pin Protocol Register
    // TPIU_PROTOCOL_MANCHESTER = SerialWire Output (UART). This is reset value
	TPI->SPPR = TPIU_PROTOCOL_UART; // Pin protocol = NRZ/UART
    
    // Formatter and Flush Control Register
    // 0x102 TPIU Packet framing enabled when bit 2 is set
    // you can use 0x100 if you only need DWT/ITM and not ETM
    // bit[8] = TrigIN = indicate a trigger on TRIGIN being asserted
    // bit[1] = EnFCont = Continuous formatting, no TRACECTL, this bit is set in reset
	TPI->FFCR = 0x100;

    /* Configure Instrumentation Trace Macroblock ITM */
            
    //enable ITM CR write privs
    // 0xC5ACCE55 = enables more write access to Control Register
	ITM->LAR = 0xC5ACCE55;	//enable ITM CR privs
	
    //Enable ITM
    //ITM Trace Control Register
    // ITM_TRACE_CTRL_SYNCENA   = Enables sync packets for TPIU
    // ITM_TRACE_CTRL_ITMENA    = Enable ITM. This is the master enable, and must be set before ITM stimulus and Trace Enable registers can be written
    // ITM_TRACE_CTRL_SWOENA    = Enable SWV behavior - count on TPIUEMIT and TPIUBAUD
	ITM->TCR = (1UL << ITM_TCR_TraceBusID_Pos)  // Trace bus ID for TPIU, it's 1 in this case
             | (1UL << ITM_TCR_DWTENA_Pos)      // Enable events from DWT
             | (1UL << ITM_TCR_SYNCENA_Pos)     // Enable sync packets
             | (1UL << ITM_TCR_TSENA_Pos)       // Enable timestamps
             | (1UL << ITM_TCR_ITMENA_Pos);     // Main enable for ITM
    

	//Set trace enable registers
    // ITM Trace Enable Register
    ITM->TER = 0xFFFFFFFF; // Enable all stimulus ports	

    return;   
}

void ITM_Print(uint32_t port, const char *p){
            /* ITM enabled */                   /* ITM Port # enabled */
    if ((ITM->TCR & ITM_TCR_ITMENA_Msk) && (ITM->TER & (1UL << port))){
        
        while(*p){
            while (ITM->PORT[0].u32 == 0);
            ITM->PORT[0].u8 = (uint8_t) *p++;
        }
        
    }
    return;    
}

void ITM_SendValue(uint32_t port, uint32_t value){
    
    if ((ITM->TCR & ITM_TCR_ITMENA_Msk) && (ITM->TER & (1UL << port))){

        while (ITM->PORT[0].u32 == 0);
        ITM->PORT[0].u32 = (uint32_t) value;

    }
    
    return;    
}

/* [] END OF FILE */
