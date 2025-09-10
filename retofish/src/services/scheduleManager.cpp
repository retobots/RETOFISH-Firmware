#include <Arduino.h>
#include <EEPROM.h>
#include <RTClib.h>
#include "services/ScheduleManager.h"

namespace SchedCfg {
  constexpr int  SLOT_COUNT          = 3;
  constexpr int  EEPROM_SIZE_BYTES   = 128;        // đủ lớn cho metadata + 3 slots
  constexpr int  EEPROM_BASE_ADDR    = 0;

  constexpr uint32_t MAGIC           = 0x53434844; // 'SCHD'
  constexpr uint16_t VERSION         = 1;

  // Phạm vi hợp lệ
  constexpr int HOUR_MIN   = 0;
  constexpr int HOUR_MAX   = 23;
  constexpr int MIN_MIN    = 0;
  constexpr int MIN_MAX    = 59;
  constexpr int DUR_MIN    = 1;
  constexpr int DUR_MAX    = 10;
}

// Gói metadata để nhận diện dữ liệu EEPROM hợp lệ
struct SchedHeader {
  uint32_t magic;
  uint16_t version;
};

// Helpers
static inline int toMinutesOfDay(int h, int m) {
  return h * 60 + m;
}
static inline int clampInt(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

ScheduleManager& ScheduleManager::getInstance() {
  static ScheduleManager instance;
  return instance;
}

void ScheduleManager::setup() {
  EEPROM.begin(SchedCfg::EEPROM_SIZE_BYTES);
  loadFromEEPROM();
}

const FeedTime* ScheduleManager::getNextFeedTime(const DateTime& now) {
  // Tìm slot ENABLED có thời điểm gần nhất kể cả qua ngày hôm sau.
  // Không phụ thuộc thứ tự _slots.
  const int nowMin = toMinutesOfDay(now.hour(), now.minute());

  int bestIdx   = -1;
  int bestDelta = 24 * 60 + 1; // lớn hơn tối đa 1 ngày

  for (int i = 0; i < SchedCfg::SLOT_COUNT; ++i) {
    const FeedTime& s = _slots[i];
    if (!s.enabled) continue;

    const int slotMin = toMinutesOfDay(s.hour, s.minute);
    // nếu slotMin >= nowMin: delta trong hôm nay, ngược lại: delta sang ngày mai
    const int delta = (slotMin >= nowMin)
                      ? (slotMin - nowMin)
                      : (slotMin + 24 * 60 - nowMin);

    if (delta < bestDelta) {
      bestDelta = delta;
      bestIdx   = i;
    }
  }

  if (bestIdx >= 0) return &_slots[bestIdx];
  return nullptr;
}

void ScheduleManager::updateSlot(int index, int hour, int minute, int duration, bool enabled) {
  if (index < 0 || index >= SchedCfg::SLOT_COUNT) return;

  // Chuẩn hóa dữ liệu
  hour     = clampInt(hour,   SchedCfg::HOUR_MIN, SchedCfg::HOUR_MAX);
  minute   = clampInt(minute, SchedCfg::MIN_MIN,  SchedCfg::MIN_MAX);
  duration = clampInt(duration, SchedCfg::DUR_MIN, SchedCfg::DUR_MAX);

  _slots[index].hour     = hour;
  _slots[index].minute   = minute;
  _slots[index].duration = duration;
  _slots[index].enabled  = enabled;

  Serial.printf("📦 Slot %d → %02d:%02d – %d (%s)\n",
                index + 1, hour, minute, duration, enabled ? "EN" : "DIS");
}

const FeedTime* ScheduleManager::getSlot(int index) const {
  if (index < 0 || index >= SchedCfg::SLOT_COUNT) return nullptr;
  return &_slots[index];
}

void ScheduleManager::saveToEEPROM() {
  // Layout:
  // [header][slot0][slot1][slot2]
  int addr = SchedCfg::EEPROM_BASE_ADDR;

  const SchedHeader header{SchedCfg::MAGIC, SchedCfg::VERSION};
  EEPROM.put(addr, header);                    addr += sizeof(SchedHeader);

  for (int i = 0; i < SchedCfg::SLOT_COUNT; ++i) {
    EEPROM.put(addr, _slots[i]);               addr += sizeof(FeedTime);
  }
  EEPROM.commit();
}

void ScheduleManager::loadFromEEPROM() {
  // Thử đọc header trước
  int addr = SchedCfg::EEPROM_BASE_ADDR;

  SchedHeader hdr{};
  EEPROM.get(addr, hdr);                       addr += sizeof(SchedHeader);

  const bool headerOk = (hdr.magic == SchedCfg::MAGIC) && (hdr.version == SchedCfg::VERSION);

  if (headerOk) {
    // Đọc 3 slot
    for (int i = 0; i < SchedCfg::SLOT_COUNT; ++i) {
      EEPROM.get(addr, _slots[i]);             addr += sizeof(FeedTime);

      // Validate mỗi slot, nếu rác thì đặt mặc định
      if (_slots[i].hour   < SchedCfg::HOUR_MIN   || _slots[i].hour   > SchedCfg::HOUR_MAX ||
          _slots[i].minute < SchedCfg::MIN_MIN    || _slots[i].minute > SchedCfg::MIN_MAX  ||
          _slots[i].duration < SchedCfg::DUR_MIN  || _slots[i].duration > SchedCfg::DUR_MAX) {
        _slots[i] = FeedTime(7 + i * 2, 0, 1, true); // 7:00, 9:00, 11:00
      }
    }
  } else {
    // Không có header hợp lệ → khởi tạo mặc định
    for (int i = 0; i < SchedCfg::SLOT_COUNT; ++i) {
      _slots[i] = FeedTime(7 + i * 2, 0, 1, true);   // 7:00, 9:00, 11:00
    }
    // Lưu header + dữ liệu mặc định ngay
    saveToEEPROM();
  }
}
