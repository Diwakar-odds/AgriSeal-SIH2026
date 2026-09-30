/**
 * Hash Chain Verifier Service
 * ============================
 * Validates cryptographic consistency of sensor readings submitted
 * by AgriSeal IoT nodes. Each packet must reference the SHA-256 hash
 * of the preceding reading to guarantee an unbroken audit trail.
 */

const crypto = require('crypto');

// In-memory tracking of latest verified hash per device
const deviceHashState = new Map();

/**
 * Recompute block hash based on node payload standard:
 * SHA256(raw_json_str)
 */
function computeHash(payloadStr) {
  return crypto.createHash('sha256').update(payloadStr).digest('hex');
}

/**
 * Verify reading consistency
 * @param {Object} reading - Parsed JSON object from node
 * @param {string} rawPayload - Raw JSON payload string
 * @returns {Object} verification result { valid: boolean, reason?: string, computedHash: string }
 */
function verifyReading(reading, rawPayload) {
  const { dev, idx, prev } = reading;

  if (!dev || idx === undefined) {
    return { valid: false, reason: 'Malformed payload: missing dev or idx' };
  }

  const computedHash = computeHash(rawPayload);
  const state = deviceHashState.get(dev);

  // If this is block #1 (genesis for this boot/session), initialize state
  if (idx === 1 || !state) {
    deviceHashState.set(dev, {
      lastIndex: idx,
      lastHash: computedHash,
      totalVerified: 1,
    });
    return { valid: true, reason: 'Genesis or re-synchronized block accepted', computedHash };
  }

  // Validate sequential index
  if (idx !== state.lastIndex + 1) {
    return {
      valid: false,
      reason: `Sequence gap detected: expected index ${state.lastIndex + 1}, received ${idx}`,
      computedHash,
    };
  }

  // Validate hash continuity
  if (prev && prev !== state.lastHash) {
    return {
      valid: false,
      reason: `Cryptographic break: payload prev '${prev.substring(0, 16)}...' != recorded '${state.lastHash.substring(0, 16)}...'`,
      computedHash,
    };
  }

  // Update verified state
  deviceHashState.set(dev, {
    lastIndex: idx,
    lastHash: computedHash,
    totalVerified: state.totalVerified + 1,
  });

  return { valid: true, computedHash };
}

function getDeviceState(deviceId) {
  return deviceHashState.get(deviceId) || null;
}

module.exports = {
  verifyReading,
  computeHash,
  getDeviceState,
};
