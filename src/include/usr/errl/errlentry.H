/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/usr/errl/errlentry.H $                            */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2018                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
#ifndef ERRLENTRY_H
#define ERRLENTRY_H
/**
 *  @file errlentry.H
 *
 *  @brief  Error Log entry object for Host Boot environment.
 *
 *  This header file contains the definition of error log entry class that
 *  is used to log errors from all firmware running in Host Boot environment.
 *
 */
/**
 * @page    errlog  Error Log
 *
 * @section Base Error Log
 *  See Host Boot Error Logging LLDD, Version 0.7  09/01/2011
 *
 * @section Adding User-Defined error blocks
 *
 *
 */

/*****************************************************************************/
// I n c l u d e s
/*****************************************************************************/
#include <stdint.h>
#include <limits.h>
#include <vector>
#include <errl/hberrltypes.H>
#include <errl/errlsctn.H>
#include <errl/errlprvt.H>
#include <errl/errluh.H>
#include <errl/errlsrc.H>
#include <errl/errlud.H>
#include <hbotcompid.H>
#include <targeting/common/target.H>
#include <attributeenums.H>

namespace ERRORLOGDISPLAY
{
class ErrLogDisplay;
}
namespace ERRORLOG
{
class ErrlEntry;
}
typedef ERRORLOG::ErrlEntry* errlHndl_t;

#include <hwas/common/hwasCallout.H>

namespace ERRORLOG
{

/*****************************************************************************/
// Forward class declarations
/*****************************************************************************/
class ErrlManager;
class ErrlUserDetailsBackTrace;

/**
 *  @brief      Host Boot Error log entry class.
 *  This class contains all of the entry management logic and necessary
 *  information to create a PEL or IPMI event type logs.
 *  A pointer to this class is known as an errlHndl_t (the error log handle)
 *  and it is the data type passed by applications on the stack to convey
 *  a failure.
 */
class ErrlEntry
{

    friend class ErrlManager;
    friend class ERRORLOGDISPLAY::ErrLogDisplay;
    friend class ErrlTest;

public:
    /** Constants to use in constructor */
    static constexpr bool ADD_SW_CALLOUT = true;
    static constexpr bool NO_SW_CALLOUT = false;
    static constexpr bool FORCE_DUMP = true;
    static constexpr bool NO_FORCE_DUMP = false;

    /**
     * @brief   ErrlEntry constructor. Builds an error log with info
     * constructed from the input.
     * Event type, Sub System, SRC type, and Terminate state are defaulted
     * to the most common values for Host Boot environment.  These values
     * can also be set to specific values by users if needed.
     * For available values, see errltypes.H
     * To understand the meaning of these values, refer to the
     * Platform Event Log and SRC PLDD at: https://mcdoc.boeblingen.de.
     * ibm.com/out/out.ViewDocument.php?documentid=1675
     *
     * The backtrace is captured as user detail data. Use removeBackTrace to
     * remove
     *
     * The order of parameters is arranged so that Host Boot error log
     * is as close to SP as possible.
     *
     * @param[in]   i_sev           Log's severity. See errltypes.H for
     *                              available values
     * @param[in]   i_modId         The module (interface) where this log is
     *                              created from.
     * @param[in]   i_reasonCode    Bits 00-07: Component Id
     *                              Bits 08-15: Reason code
     * @param[in]   i_user1         64 bits of user data which are placed
     *                              in the primary SRC
     * @param[in]   i_user2         64 bits of user data which are placed
     *                              in the primary SRC
     * @param[in]   i_hbSwError     if true, will automatically add a HIGH
     *                              severity callout for EPUB_PRC_HB_CODE
     * @param[in]   i_hbDump        if true, will flag error log as a candidate
     *                              for an HB dump if log is committed and is
     *                              used as status for a shutdown
     * @return  None
     */
    ErrlEntry(const errlSeverity_t i_sev,
              const uint8_t i_modId,
              const uint16_t i_reasonCode,
              const uint64_t i_user1 = 0,
              const uint64_t i_user2 = 0,
              const bool i_hbSwError = ErrlEntry::NO_SW_CALLOUT,
              const bool i_hbDump = ErrlEntry::NO_FORCE_DUMP );

    /**
     * @brief   Destructor
     *
     * Releases all resources owned by the handle.  If the log has not
     * been committed, it effectively aborts the log.
     * All logs not committed must be deleted to avoid a resource leak.
     * Committing an error log via ErrlManager will cause ErrlManager
     * to delete the log after saving it.
     *
     * @return  None
     *
     */
    ~ErrlEntry();

    /**
     *  @brief Read the log's severity
     *  The severity of a log determines how severe the
     *  problem is perceived to be.  The severity is
     *  initially set by the parameter in the constructor
     *  and can be changed at any point thereafter.
     *  See errl/errltypes.H
     *
     *  @return Current Log Severity
     *
     */
    errlSeverity_t sev() const { return iv_User.iv_severity; }


    /**
     *  @brief Set the log's severity
     *  The severity of a log determines how severe the
     *  problem is perceived to be.  The severity is
     *  initially set by the parameter in the constructor
     *  and can be changed at any point thereafter, unless the
     *  the severity has been set as "final".
     *
     *  @param[in]  i_sev Severity. See errl/errltypes.H
     *
     *  @return     void
     */
    void setSev(const errlSeverity_t i_sev)
    {
        if (false == iv_sevFinal) // update severity as long as not set "final"
        {
            iv_User.iv_severity = i_sev;
        }
    }

    /**
     *  @brief Set the log's severity "final".
     *  The severity being set will be set as "final" such that it can
     *  not be changed again. Setting a severity as "final" can be used
     *  in the less frequent case that a severity should not be
     *  changed by code at a higher level.
     *
     *  @param[in]  i_sev    Severity. See errl/errltypes.H
     *  @param[in]  i_final  true: If not already "final", sets severity and
     *                             marks the severity as "final"
     *                       false: If not "final", sets severity
     *
     *  @return     void
     */
    void setSev(const errlSeverity_t i_sev,
                bool  i_final)
    {

        if (false == iv_sevFinal) // as long as not already set as "final"
        {
            iv_User.iv_severity = i_sev;  // set severity
            iv_sevFinal = i_final;        // set whether severity is "final"
        }
    }

    /**
     *  @brief Get reason code. See errl/errlreasoncodes.H
     *
     *  @return reason code
     */
    uint16_t reasonCode() const;

    /**
     *  @brief Set reason code
     *
     *  @param[in]  i_reasonCode   The reason code.  The upper byte
     *                             of a reason code is the component ID.
     *                             See errl/errlreasoncodes.H
     *
     *  @return     void
     */
    void setReasonCode( const uint16_t i_reasonCode );

    /**
     *  @brief Get module id.  See errl/errlreasoncodes.H
     *
     *  @return module id
     */
    uint8_t moduleId() const;



    /**
     *  @brief Set module id
     *
     *  @param[in]  i_moduleId  The module ID. See errl/errlreasoncodes.H
     *
     *  @return     void
     */
    void setModuleId( const uint8_t i_moduleId );

    /**
     *  @brief Get the unique error log identifier (EID) of the error log.
     *
     *  @return The error log ID of the error log.
     */
    uint32_t eid() const;

    /**
     *  @brief Get the platform log identifier (PLID) of the error log.
     *  In legacy FSP, you could have one platform log ID such that a series
     *  of entry IDs (EIDs) related to a single PLID.
     *
     *  @return The platform log ID of the error log.
     */
    uint32_t plid() const;

    /**
     *  @brief Set the platform log id for this error log.
     *
     *  @param[in]  i_plid  PLID for error log
     *
     *  @return void
     */
    void plid( uint32_t i_plid);

    /**
     *  @brief Get the event type of the error log.
     *         See errl/errltypes.H
     *  @return errlEventType_t
     *
     */
    errlEventType_t eventType() const { return iv_User.iv_etype; }

    /**
     *  @brief Set the log's event type
     *
     *  @param[in]  i_eventType  Event type. See errl/errltypes.H
     *
     *  @return void
     *
     */
    void setEventType(const errlEventType_t i_eventType)
    {
        iv_User.iv_etype = i_eventType;
    }

    /**
     *  @brief Returns the log's sub system. See errl/errltypes.H
     *
     *  @return epubSubSystem_t
     *
     */
    epubSubSystem_t subSys() const { return iv_User.iv_ssid; }

   /**
     *  @brief Set the log's ePub sub system type.
     *
     *  @param[in]  i_subSys   Subsystem type. See errl/errltypes.H
     *
     *  @return void
     */
    void setSubSys(const epubSubSystem_t i_subSys)
    {
        // hb has ssid defined in two places
        iv_User.iv_ssid = i_subSys;
        iv_Src.iv_ssid  = i_subSys;
    }


    /**
     *  @brief Returns the log's SRC type. See errl/errltypes.H
     *
     *  @return srcType_t
     *
     */
    srcType_t srcType() const;

   /**
     *  @brief Set the log's SRC type
     *
     *  @param[in]  i_srcType   The SRC type for this error log. See
     *                          errl/errltypes.H
     *
     *  @return void
     */
    void setSrcType(const srcType_t i_srcType);

    /**
     *  @brief Returns the log's terminate state type. See
     *  errl/errltypes.H
     *
     *  @return errlTermState_t
     *
     */
    errlTermState_t termState() const;

    /**
     *  @brief Set the log's Terminating state
     *
     *  @param[in] i_termState  See errl/errltypes.H
     *
     *  @return void
     *
     */
    void setTermState(const errlTermState_t i_termState);

    /**
     * @brief Helper function to determine if log is a terminating error
     *
     *
     * @return  returns true if error will result in system termination.
     */
    bool isTerminateLog() const;

    /**
     *  @brief Add data to the iv_Src user data words.
     *
     *  @param[in] i_data - information to add to the user data word 1.
     *
     *  @return void
     *
     */
    void addUserData1( const uint64_t i_data );
    /**
     *  @brief Add data to the iv_Src user data words.
     *
     *  @param[in] i_data - information to add to the user data word 2.
     *
     *  @return void
     *
     */
    void addUserData2( const uint64_t i_data );

    /**
     *  @brief set Deconfigure and GARD bits in Hex Word 5
     *
     *  @return void
     *
     */
    void setDeconfigBit();
    void setGardBit();

    /**
     *  @brief Return iv_Src user data words.
     *
     *  @return data1 word from SRC
     *
     */
    uint64_t getUserData1() const;

    /**
     *  @brief Return iv_Src user data words.
     *
     *  @return data2 word from SRC
     *
     */
    uint64_t getUserData2() const;


   /**
     * @brief Allows the caller to add a chunk of FFDC data in a log
     *
     * @param[in]   i_compId         Component Id of the caller
     * @param[in]   i_dataPtr        Pointer to FFDC data
     * @param[in]   i_ffdcLen        Length of the data in bytes
     * @param[in]   i_ffdcVer        A user supplied identifier which
     *                               classifies the data
     * @param[in]   i_ffdcSubSect    A user supplied sub section identifier
     *                               which classifies the data.  This in
     *                               conjunction with the version can be used
     *                               to decode the data.
     * @param[in]   i_merge          A boolean value (defaults to false if
     *                               parameter is not specified) to indicate
     *                               whether this buffer is merged with the
     *                               previous section iff it matches.
     *
     * @return Pointer to FFDC section if successfully added.
     *         NULL if fails
     */
    ErrlUD * addFFDC(const compId_t i_compId,
                     const void * i_dataPtr,
                     const uint32_t i_ffdcLen,
                     const uint8_t i_ffdcVer,
                     const uint8_t i_ffdcSubSect,
                     bool i_merge = false);

    /**
     * @brief Append more data to an FFDC section.
     *
     * @param[in]   i_pErrlUD        Pointer to the user-define section
     *                               to add data to.  This pointer is
     *                               returned when addFFDC
     *                               function is called earlier.
     * @param[in]   i_dataPtr        Points to data block to be added
     * @param[in]   i_dataLen        Length of data in bytes
     *
     * @return void
     */
    void appendToFFDC( ErrlUD * i_pErrlUD,
                       const void *i_dataPtr,
                       const uint32_t i_dataLen);


    /**
     *  @brief Collect component trace
     *  The trace buffer named is collected and added to the error
     *  log.  The amount of traces is controlled by the i_max parameter.
     *  When zero, or left to default, the full trace buffer is copied.
     *  Otherwise, i_max size must be big enough to hold a trace buffer
     *  header (40 bytes) plus some trace data. For example, a trace
     *  entry with n bytes of data is n+28 bytes in size.
     *
     *  Note that component names given in hbotcompid.H do not necessarily
     *  map to the names of trace buffers created by that component.
     *  Trace buffer names are case insensitive.
     *
     *  @param[in]  i_name          Trace buffer name
     *  @param[in]  i_max           Size of trace to capture.
     *
     *  @note: 'i_max = 0' implies to collect all of the trace available for
     *         that component.  This is actually dangerous in Hostboot because
     *         our trace buffer are not hard-limited.  The default of this
     *         function will be 1K to keep us at a reasonable limit.
     *
     *  @return A Boolean indication of success.  False likely means
     *  the trace buffer name given is not found. However, check the ERRL
     *  trace buffer for the cause of the failure.
     */
    bool collectTrace(const char i_name[],
                      const uint64_t i_max = KILOBYTE);

    /**
     *  @brief Remove the back trace user detail data
     *  When an error log is constructed, the back trace is automatically
     *  captured. This function removes the backtrace. This should be used when
     *  a caller knows that the backtrace is of no use and wants to limit the
     *  size of the error log.
     */
    void removeBackTrace();

    /**
     *  @brief Add a clock callout.
     *  The i_target is used to identify the actual clock to callout because
     *  some systems have multiple clocks of a given type. This target is
     *  usually a chip target, but can possibly be a unit, depending on
     *  the clock type.
     *
     *  @param[in]  i_target        The hardware target fed by the clock
     *  @param[in]  i_clockType     The hardware clock type
     *  @param[in]  i_priority      Priority of the callout
     *  @param[in]  i_deconfigState Enum indicating whether the failing part
     *                              should be deconfigured.
     *  @param[in]  i_gardErrorType To indicate what type of failure occurred
     *
     *  @return void
     */
    void addClockCallout(const TARGETING::Target *i_target,
                       const HWAS::clockTypeEnum i_clockType,
                       const HWAS::callOutPriority i_priority,
                       const HWAS::DeconfigEnum
                                   i_deconfigState = HWAS::NO_DECONFIG,
                       const HWAS::GARD_ErrorType
                                   i_gardErrorType = HWAS::GARD_NULL);
    /**
     *  @brief Add a part callout.
     *  The i_target is used to identify the associated target. This target
     *  may not be a FRU
     *
     *  @param[in]  i_target        The hardware target that owns this part
     *  @param[in]  i_partType      The hardware part type
     *  @param[in]  i_priority      Priority of the callout
     *  @param[in]  i_deconfigState Enum indicating whether the failing part
     *                              should be deconfigured.
     *  @param[in]  i_gardErrorType To indicate what type of failure occurred
     *
     *  @return void
     */
    void addPartCallout(const TARGETING::Target *i_target,
                       const HWAS::partTypeEnum i_partType,
                       const HWAS::callOutPriority i_priority,
                       const HWAS::DeconfigEnum
                                   i_deconfigState = HWAS::NO_DECONFIG,
                       const HWAS::GARD_ErrorType
                                   i_gardErrorType = HWAS::GARD_NULL);


    /**
     *  @brief Add a bus callout
     *
     *  @param[in]  i_target1       The hardware bus endpoint target1
     *  @param[in]  i_target2       The hardware bus endpoint target2
     *  @param[in]  i_busType       The hardware bus type
     *  @param[in]  i_priority      Priority of the callout
     *  @param[in]  i_flag          Extra flag for callout structure
     *
     *  @return void
     */
    void addBusCallout(const TARGETING::Target *i_target1,
                       const TARGETING::Target *i_target2,
                       const HWAS::busTypeEnum i_busType,
                       const HWAS::callOutPriority i_priority,
                       const HWAS::CalloutFlag_t i_flags = HWAS::FLAG_NONE);

    /**
     *  @brief Add a bus callout with entity paths
     *
     *  @param[in]  i_target1       The PHYS_PATH of HW bus endpoint target1
     *  @param[in]  i_target2       The PHYS_PATH of HW bus endpoint target2
     *  @param[in]  i_busType       The hardware bus type
     *  @param[in]  i_priority      Priority of the callout
     *  @param[in]  i_flag          Extra flag for callout structure
     *
     *  @return void
     */
    void addBusCallout(const TARGETING::EntityPath & i_target1,
                       const TARGETING::EntityPath & i_target2,
                       HWAS::busTypeEnum i_busType,
                       HWAS::callOutPriority i_priority,
                       const HWAS::CalloutFlag_t i_flags = HWAS::FLAG_NONE);


    /**
     *  @brief Add a hardware callout
     *
     *  @param[in]  i_target        The hardware target
     *  @param[in]  i_priority      Priority of the callout
     *  @param [in] i_deconfigState Enum indicating whether the failing part
     *               should be deconfigured.
     *  @param [in] i_gardErrorType to indicate what type of failure occurred
     *
     *  @return void
     */
    void addHwCallout(const TARGETING::Target *i_target,
                        const HWAS::callOutPriority i_priority,
                        const HWAS::DeconfigEnum i_deconfigState,
                        const HWAS::GARD_ErrorType i_gardErrorType);

    /**
     *  @brief Add a procedure ( software ) callout
     *  Adds the given service procedure to the list
     *  of callouts for the log
     *
     *  @param[in]  i_procedure     Procedure identifier.
     *  @param[in]  i_priority      Priority of the callout
     *
     *  @return void
     */
    void addProcedureCallout(const HWAS::epubProcedureID i_procedure,
                             const HWAS::callOutPriority i_priority);


    /**
     *  @brief Add a special sensor callout
     *  Adds the given sensor to the list of callouts for the log
     *
     *  @param[in]  i_sensorID      Sensor ID
     *  @param[in]  i_sensorType    Type of sensor being added
     *  @param[in]  i_priority      Priority of the callout
     *
     *  @return void
     */
    void addSensorCallout(const uint32_t i_sensorID,
                          const HWAS::sensorTypeEnum i_sensorType,
                          const HWAS::callOutPriority i_priority);

    /**
     *  @brief Add an i2c device callout. Use this callout type for i2c devices
     *         that do not have target associated with them.
     *
     *  @param[in]  i_i2cMaster     The i2c master target. Cannot be nullptr.
                                    Cannot be the master sentinel.
     *  @param[in]  i_engine        The i2c device engine
     *  @param[in]  i_port          The i2c device port
     *  @param[in]  i_address       The i2c device address
     *  @param[in]  i_priority      Priority of the callout
     *
     *  @return void
     */
    void addI2cDeviceCallout(const TARGETING::Target *i_i2cMaster,
                             const uint8_t i_engine,
                             const uint8_t i_port,
                             const uint8_t i_address,
                             const HWAS::callOutPriority i_priority);


    /**
     * @brief  Import flattened error log
     *
     * @param[in] i_buffer, pointer to flat data
     * @param[in] i_len length of flattened data
     * @return Status  0==SUCCESS
     */
    uint64_t unflatten( const void * i_buffer, uint64_t i_len);

    /**
    * @brief Return the list of User Detail sections
    *
    * @param[in] i_compId Component id associated with the data to return
    * @param[in] i_subSect Subsection id associated with the data to return
    *
    *            //NOTE: You can pass COMP_ID or subsect 0 into this
    *                    function for wildcard
    *
    * @return vector of void pointers that point to the data buffer inside
    *         of a user details section
    */
    std::vector<void*> getUDSections(compId_t i_compId, uint8_t i_subSect);

    /**
     * @brief set iv_eselCallhomeInfoEvent
     *
     *      When true, send this error as a special callhome
     *                 type of eSEL to the BMC
     *
     * @return nothing
     */
    void setEselCallhomeInfoEvent(bool i_valid);

    /**
     * @brief get iv_eselCallhomeInfoEvent
     *
     * @return true if this log should result in a callhome event type eSEL,
     *         false otherwise
     */
    bool getEselCallhomeInfoEvent();

    /**
     * @brief get iv_doHbDump
     *
     * @return true if this log should result in a HB dump during a shutdown,
     *         false otherwise
     */
    bool getDoHbDump();

private:

     /**
     *  @brief Set the failing subsystem ID in the error log
     *         based on the highest priority callout.
     *
     */
    void setSubSystemIdBasedOnCallouts();

    /**
     *  @brief maps the target type to corresponding subsystem ID
     *
     *  @param[in]  i_target   Target Type.
     *
     *  @return     subsystem ID
     */
    epubSubSystem_t getSubSystem( TARGETING::TYPE i_target ) const;

    /**
     *  @brief maps the procedure to a corresponding subsystem ID
     *
     *  @param[in]  i_procedureId   procedure.
     *
     *  @return     subsystem ID
     */
    epubSubSystem_t getSubSystem( HWAS::epubProcedureID i_procedureId ) const;

    /**
     *  @brief maps a bus type to a subsystem ID
     *
     *  @param[in]  i_busType   Bus Type.
     *
     *  @return     subsystem ID
     */
    epubSubSystem_t getSubSystem( HWAS::busTypeEnum i_busType ) const;

    /**
     *  @brief maps a clock type to a subsystem ID
     *
     *  @param[in]  i_clockType   Bus Type.
     *
     *  @return     subsystem ID
     */
    epubSubSystem_t getSubSystem( HWAS::clockTypeEnum i_clockType ) const;

    /**
     *  @brief maps a part type to a subsystem ID
     *
     *  @param[in]  i_partType   Bus Type.
     *
     *  @return     subsystem ID
     */
    epubSubSystem_t getSubSystem( HWAS::partTypeEnum i_partType ) const;

    /**
     *  @brief maps a sensor type to a subsystem ID
     *
     *  @param[in]  i_sensorType   Sensor Type.
     *
     *  @return     subsystem ID
     */
    epubSubSystem_t getSubSystem( HWAS::sensorTypeEnum i_sensorType ) const;

    /**
     * @brief The ErrlManager will call here to ask the
     * ErrlEntry to assign the time of commit, and to assign
     * the commiter's comp ID.
     * The ErrlManager is the primary user of this call.
     *
     */
    void commit( compId_t i_commiterComp);


    /**
     * @brief The ErrlManager will call here to ask the
     * ErrlEntry to handle deconfigure and GARD callouts
     * The ErrlManager is the primary user of this call.
     *
     */
    void processCallout();


    /**
     * @brief The ErrlManager will call here to ask the
     * ErrlEntry to handle deferred deconfigure callouts
     * The ErrlManager is the primary user of this call.
     *
     */
    void deferredDeconfigure();


    /**
     * @brief Compute the flattened size of an error log.  It is typical
     * to call this function for the size of buffer required, then
     * allocate a buffer, then call flatten().
     * The ErrlManager is the primary user of this call.
     *
     * @return Size in bytes of the flattened data.
     */
    uint64_t flattenedSize();


    /**
     * @brief sort compare function, to be used when flattening an
     * error log
     *
     * @param[in]      i_ud1 - first user-defined section to compare
     * @param[in]      i_ud2 - second user-defined section to compare
     *
     * @return  true if i_ud1 should be first, false otherwise
     */
    static bool udSort(ErrlUD *i_ud1, ErrlUD *i_ud2);

    /**
     * @brief Flatten the data to caller's buffer. The resulting flat
     * data will be in PEL (platform event log) format.
     * The ErrlManager is the primary user of this call.
     *
     * @param[in,out]  o_buffer   Points to data block to be filled
     * @param[in]      i_cbBuffer Count of bytes in buffer supplied
     * @param[in]      i_truncate true: truncate the data
     *                              false: error if buffer too small
     *                  NOTE: PH, UH and PS MUST fit
     *
     * @return  Count of bytes copied to caller's
     *          buffer or else zero if it does not fit and i_truncate==false.
     */
    uint64_t flatten( void * o_buffer, const uint64_t i_cbBuffer,
            const bool i_truncate = false);

    /**
     * @brief Called by commit(), this function creates a
     * user details section which contains the Hostboot
     * build ID and adds this to the current error log being
     * processed.
     */
    void addHbBuildId();

#ifdef CONFIG_BMC_IPMI
    /**
     * @brief called by addHwCallout to retrieve the serial and part number
     * from the current target. If the current target does not contain
     * ATTR_PART_NUMBER or ATTR_SERIAL_NUMBER, find the first parent that does
     * and adds the attribute to the error log.
     *
     * @param[in]       i_target        The target to get the numbers for
     */
    void addPartAndSerialNumbersToErrLog(const TARGETING::Target * i_target);

    /**
     * @brief called by addHwCallout to retrieve the FRU ID and sensor ID
     * from the current target. If the current target does not contain
     * ATTR_FRU_ID, find the first parent that does and add the attribute
     * to the error log.
     *
     * @param[in]       i_target        The target to get the IDs for
     * @param[in]       i_priority      This target's callout priority
     */
    void addSensorDataToErrLog(TARGETING::Target * i_target,
                               HWAS::callOutPriority i_priority );
#endif

    /**
     * @brief set iv_skipShowingLogs
     *
     * @return nothing
     */
    void setSkipShowingLog(bool i_showLog);

    /**
     * @brief get iv_skipShowingLogs
     *
     * @return true if the log should be skipped, false otherwise
     */
    bool getSkipShowingLog();


    /**
     * @brief Sets internal flag to indicate if this log should be
     *    saved to PNOR and sent to the BMC
     *
     * @return nothing
     */
    void checkHiddenLogsEnable( );

    /**
     * @brief Check if the severity of this log indicates it is
     *   customer visible, note this ignores any override flags that
     *   might change standard behavior
     *
     *  INFORMATIONAL = false
     *  RECOVERED = false
     *  PREDICTIVE = true
     *  UNRECOVERABLE = true
     *  CRITICAL_SYS_TERM = true
     *
     * @return true if log is visible
     */
    bool isSevVisible( void );


    /**
      * @brief   Disabled copy constructor and assignment operator
      */
    ErrlEntry(const ErrlEntry& i_right);
    ErrlEntry& operator=(const ErrlEntry& i_right);

private:
    // Data Members
    ErrlPrvt        iv_Private;    // private header object
    ErrlUH          iv_User;       // user header object
    ErrlSrc         iv_Src;        // primary SRC section
    std::vector<ErrlUD*> iv_SectionVector;  // list of user defined sections
    std::vector<uint64_t> iv_btAddrs;    // list of addresses for the backtrace
    ErrlUserDetailsBackTrace * iv_pBackTrace; // Pointer to back trace

    // termstate will not be flattend into PEL, its only used by
    // hostboot.
    errlTermState_t iv_termState;

   // when true, the severity has been set "final" and can not be changed.
   bool iv_sevFinal;

   //when true, the current error log will not be saved to PNOR, sent to the
   //BMC, or displayed in the console
   bool iv_skipShowingLog;

   // when true, send this special type of eSEL to the BMC
   // This is used to send OCC informational errors up to BMC
   bool iv_eselCallhomeInfoEvent;

   // when true, if error log is committed, then the error log is tracked so
   // doShutdown can request that a HB dump be done.
   bool iv_doHbDump;
};





//-----------------------------------------------------------------------
// In-line functions
//-----------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

inline uint32_t ErrlEntry::eid() const
{
    return iv_Private.iv_eid;
}

inline uint32_t ErrlEntry::plid() const
{
    return iv_Private.iv_plid;
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline void ErrlEntry::plid( uint32_t i_plid)
{
    iv_Private.iv_plid = i_plid;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline uint8_t ErrlEntry::moduleId() const
{
    return iv_Src.iv_modId;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline void ErrlEntry::setModuleId( const uint8_t i_moduleId )
{
    iv_Src.iv_modId = i_moduleId;
    return;
}




////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline errlTermState_t ErrlEntry::termState() const
{
    return iv_termState;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline void ErrlEntry::setTermState(const errlTermState_t i_termState)
{
    iv_termState = i_termState;
    return;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline void ErrlEntry::setReasonCode( const uint16_t i_reasonCode )
{
    iv_Src.iv_reasonCode = i_reasonCode;
    return;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline void ErrlEntry::addUserData1( const uint64_t i_data )
{
    iv_Src.iv_user1 = i_data;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline void ErrlEntry::addUserData2( const uint64_t i_data )
{
    iv_Src.iv_user2 = i_data;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline void ErrlEntry::setDeconfigBit()
{
    iv_Src.iv_deconfig = true;
}
inline void ErrlEntry::setGardBit()
{
    iv_Src.iv_gard = true;
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline uint64_t ErrlEntry::getUserData1() const
{
    return iv_Src.iv_user1;
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline uint64_t ErrlEntry::getUserData2() const
{
    return iv_Src.iv_user2;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline uint16_t ErrlEntry::reasonCode() const
{
    return iv_Src.iv_reasonCode;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline srcType_t ErrlEntry::srcType() const
{
    return iv_Src.iv_srcType;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline void ErrlEntry::setSrcType(const srcType_t i_srcType)
{
    iv_Src.iv_srcType = i_srcType;
    return;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline void ErrlEntry::setSkipShowingLog(bool i_showLog)
{
    iv_skipShowingLog = i_showLog;
    return;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline bool ErrlEntry::getSkipShowingLog()
{
    return iv_skipShowingLog;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline void ErrlEntry::setEselCallhomeInfoEvent(bool i_valid)
{
    iv_eselCallhomeInfoEvent = i_valid;
    return;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline bool ErrlEntry::getEselCallhomeInfoEvent()
{
    return iv_eselCallhomeInfoEvent;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline bool ErrlEntry::getDoHbDump()
{
    return iv_doHbDump;
}

} // End namespace


/**
 * Retrieve the RC inside an error log, or return zero if the log is NULL
 */
#define ERRL_GETRC_SAFE(errhdl) (errhdl == NULL ? 0 : errhdl->reasonCode())

/**
 * Retrieve the PLID inside an error log, or return zero if the log is NULL
 */
#define ERRL_GETPLID_SAFE(errhdl) (errhdl == NULL ? 0 : errhdl->plid())

/**
 * Retrieve the EID inside an error log, or return zero if the log is NULL
 */
#define ERRL_GETEID_SAFE(errhdl) (errhdl == NULL ? 0 : errhdl->eid())

#endif //ERRLENTRY_H
