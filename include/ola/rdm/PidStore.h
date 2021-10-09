/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * PidStore.h
 * Holds information about RDM PIDs.
 * Copyright (C) 2011 Simon Newton
 */

/**
 * @addtogroup rdm_pids
 * @{
 * @file PidStore.h
 * @brief Holds information about RDM PIDs.
 * @}
 */

#ifndef INCLUDE_OLA_RDM_PIDSTORE_H_
#define INCLUDE_OLA_RDM_PIDSTORE_H_

#include <stdint.h>
#include <ola/messaging/Descriptor.h>
#include <ola/base/Macro.h>
#include <istream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ola {
namespace rdm {

class PidStore;
class PidDescriptor;

// The following % before Device is to stop Doxygen interpreting it as a class
/**
 * @brief The root of the RDM parameter descriptor store.
 *
 * The root parameter store holds the ESTA (formerly known as PLASA, formerly
 * known as ESTA) parameters as well as any manufacturer defined parameters.
 * Parameter definitions are loaded from .proto files, which are generated by
 * the http://rdm.openlighting.org site.
 *
 * Each parameter has an 16bit identifier (PID).
 *
 * ESTA PIDs are those defined by the E1.X series of documents. To date this
 * includes:
 *   - E1.20, Remote %Device Management.
 *   - E1.37-1, Additional Message Sets for Dimmers.
 *   - E1.37-2, Additional Message Sets for IPv4 & DNS Configuration.
 *
 * An overrides.proto file can be used as a local system override of any PID
 * data. This allows manufacturers to specify their own manufacturer specific
 * commands and for testing of draft PIDs.
 */
class RootPidStore {
 public:
  typedef std::map<uint16_t, const PidStore*> ManufacturerMap;

  /**
   * @brief Create a new RootPidStore.
   *
   * Most code shouldn't have to use this. Use RootPidStore::LoadFromFile or
   * RootPidStore::LoadFromDirectory instead.
   */
  RootPidStore(const PidStore *esta_store,
               const ManufacturerMap &manufacturer_stores,
               uint64_t version = 0)
      : m_esta_store(esta_store),
        m_manufacturer_store(manufacturer_stores),
        m_version(version) {
  }

  ~RootPidStore();

  /**
   * @brief The version of the RDM parameter data.
   * @returns The version of the parameter data. A higher number is a more
   * recent version.
   */
  uint64_t Version() const { return m_version; }

  /**
   * @brief Return the PidStore for ESTA (PLASA) parameters.
   * @returns the PidStore for the ESTA parameters. The pointer is valid for
   * the lifetime of the RootPidStore.
   */
  const PidStore *EstaStore() const {
    return m_esta_store.get();
  }

  /**
   * @brief Return the PidStore for a manufacturer.
   * @param esta_id the manufacturer id.
   * @returns A pointer to a PidStore or NULL if there were no parameters for
   * this manufacturer.
   */
  const PidStore *ManufacturerStore(uint16_t esta_id) const;

  /**
   * @brief Lookup an ESTA-defined parameter by name.
   * @param pid_name the name of the parameter.
   * @return a PidDescriptor or NULL if the parameter wasn't found.
   */
  const PidDescriptor *GetDescriptor(const std::string &pid_name) const;

  /**
   * @brief Lookup a parameter by name in both the ESTA and the specified
   * manufacturer store.
   * @param pid_name the name of the parameter to look for.
   * @param manufacturer_id the ESTA id of the manufacturer.
   * @return a PidDescriptor or NULL if the parameter wasn't found.
   */
  const PidDescriptor *GetDescriptor(const std::string &pid_name,
                                     uint16_t manufacturer_id) const;

  /**
   * @brief Look up a ESTA-defined parameter by PID.
   * @param pid_value the PID to lookup.
   * @return a PidDescriptor or NULL if the parameter wasn't found.
   */
  const PidDescriptor *GetDescriptor(uint16_t pid_value) const;

  /**
   * @brief Lookup a parameter by PID in both the ESTA and the specified
   * manufacturer store.
   * @param pid_value the pid to lookup
   * @param manufacturer_id the ESTA id of the manufacturer.
   * @return a PidDescriptor or NULL if the parameter wasn't found.
   */
  const PidDescriptor *GetDescriptor(uint16_t pid_value,
                                     uint16_t manufacturer_id) const;

  /**
   * @brief Load a RootPidStore from a file.
   * @param file the file to load
   * @param validate whether to perform validation on the data. Validation can
   * be turned off for faster load times.
   */
  static const RootPidStore *LoadFromFile(const std::string &file,
                                          bool validate = true);

  /**
   * @brief Load a RootPidStore from a directory.
   * @param directory the directory containing the PID data. If directory is
   * empty, the installed location will be used.
   * @param validate whether to perform validation on the data. Validation can
   * be turned off for faster load times.
   */
  static const RootPidStore *LoadFromDirectory(const std::string &directory,
                                               bool validate = true);

  /**
   * @brief Returns the location of the installed PID data.
   * @returns the directory where the pid data was installed.
   */
  static const std::string DataLocation();

 private:
  std::auto_ptr<const PidStore> m_esta_store;
  ManufacturerMap m_manufacturer_store;
  uint64_t m_version;

  const PidDescriptor *InternalESTANameLookup(
      const std::string &pid_name) const;

  DISALLOW_COPY_AND_ASSIGN(RootPidStore);
};


/**
 * @brief Holds the PidDescriptors for a single manufacturer.
 */
class PidStore {
 public:
  /**
   * @brief Create a new PidStore with the given PidDescriptors.
   * @param pids a list of PidDescriptors to use.
   * @pre the names and values for the pids in the vector are unique.
   * @note Most code shouldn't have to use this. Call
   * RootPidStore::LoadFromFile() or RootPidStore::LoadFromDirectory()
   * instead.
   */
  explicit PidStore(const std::vector<const PidDescriptor*> &pids);

  /**
   * @brief Clean up
   */
  ~PidStore();

  /**
   * @brief The number of PidDescriptors in this store.
   * @returns the number of PidDescriptors in this store.
   */
  unsigned int PidCount() const { return m_pid_by_value.size(); }

  /**
   * @brief Return a list of all PidDescriptors.
   * @param[out] pids a vector which is populated with a list of
   * PidDescriptors.
   *
   * The pointers returned are valid for the life of the PidStore object.
   */
  void AllPids(std::vector<const PidDescriptor*> *pids) const;

  /**
   * @brief Lookup a PidDescriptor by PID.
   * @param pid_value the PID to lookup.
   * @return a PidDescriptor or NULL if the parameter wasn't found.
   */
  const PidDescriptor *LookupPID(uint16_t pid_value) const;

  /**
   * @brief Lookup a PidDescriptor by parameter name.
   * @param pid_name the name of the parameter to look for.
   * @return a PidDescriptor or NULL if the parameter wasn't found.
   */
  const PidDescriptor *LookupPID(const std::string &pid_name) const;

 private:
  typedef std::map<uint16_t, const PidDescriptor*> PidMap;
  typedef std::map<std::string, const PidDescriptor*> PidNameMap;
  PidMap m_pid_by_value;
  PidNameMap m_pid_by_name;

  DISALLOW_COPY_AND_ASSIGN(PidStore);
};


/**
 * Contains the descriptors for the GET/SET Requests & Responses for a single
 * PID.
 */
class PidDescriptor {
 public:
  // TODO(simon): use the enums from the Pids.proto instead of duplicating
  // here.
  typedef enum {
    ROOT_DEVICE,  // 0 only
    ANY_SUB_DEVICE,  // 0 - 512 or ALL_DEVICES
    NON_BROADCAST_SUB_DEVICE,  // 0 - 512
    SPECIFIC_SUB_DEVICE,  // 1- 512
  } sub_device_validator;

  PidDescriptor(const std::string &name,
                uint16_t value,
                const ola::messaging::Descriptor *get_request,
                const ola::messaging::Descriptor *get_response,
                const ola::messaging::Descriptor *set_request,
                const ola::messaging::Descriptor *set_response,
                sub_device_validator get_sub_device_range,
                sub_device_validator set_sub_device_range)
      : m_name(name),
        m_pid_value(value),
        m_get_request(get_request),
        m_get_response(get_response),
        m_set_request(set_request),
        m_set_response(set_response),
        m_get_subdevice_range(get_sub_device_range),
        m_set_subdevice_range(set_sub_device_range) {
  }
  ~PidDescriptor();

  const std::string &Name() const { return m_name; }
  uint16_t Value() const { return m_pid_value; }
  const ola::messaging::Descriptor *GetRequest() const { return m_get_request; }
  const ola::messaging::Descriptor *GetResponse() const {
    return m_get_response;
  }
  const ola::messaging::Descriptor *SetRequest() const { return m_set_request; }
  const ola::messaging::Descriptor *SetResponse() const {
    return m_set_response;
  }

  bool IsGetValid(uint16_t sub_device) const;
  bool IsSetValid(uint16_t sub_device) const;

  static bool OrderByName(const PidDescriptor* a, const PidDescriptor* b);

 private:
  const std::string m_name;
  uint16_t m_pid_value;
  const ola::messaging::Descriptor *m_get_request;
  const ola::messaging::Descriptor *m_get_response;
  const ola::messaging::Descriptor *m_set_request;
  const ola::messaging::Descriptor *m_set_response;
  sub_device_validator m_get_subdevice_range;
  sub_device_validator m_set_subdevice_range;

  bool RequestValid(uint16_t sub_device,
                    const sub_device_validator &validator) const;

  DISALLOW_COPY_AND_ASSIGN(PidDescriptor);
};
}  // namespace rdm
}  // namespace ola
#endif  // INCLUDE_OLA_RDM_PIDSTORE_H_
