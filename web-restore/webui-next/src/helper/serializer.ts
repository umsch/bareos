// Helper function to convert Uint8Array to Base64
function uint8ArrayToBase64(uint8Array: Uint8Array) {
  return btoa(String.fromCharCode.apply(null, Array.from(uint8Array)))
}

// Helper function to convert Base64 back to Uint8Array
function base64ToUint8Array(base64: string) {
  const binaryString = atob(base64)
  const len = binaryString.length
  const bytes = new Uint8Array(len)
  for (let i = 0; i < len; i++) {
    bytes[i] = binaryString.charCodeAt(i)
  }
  return bytes
}

// Combined serialization
function serializeWithUint8ArrayAndBigInt(obj: object) {
  return JSON.stringify(obj, (key, value) => {
    if (value instanceof Uint8Array) {
      return { __type: 'Uint8Array', data: uint8ArrayToBase64(value) }
    }
    if (typeof value === 'bigint') {
      return { __type: 'BigInt', data: value.toString() }
    }
    return value
  })
}

// Combined deserialization
function deserializeWithUint8ArrayAndBigInt(jsonStr: string) {
  return JSON.parse(jsonStr, (key, value) => {
    if (value && value.__type === 'Uint8Array') {
      return base64ToUint8Array(value.data)
    }
    if (value && value.__type === 'BigInt') {
      return BigInt(value.data)
    }
    return value
  })
}

export { serializeWithUint8ArrayAndBigInt, deserializeWithUint8ArrayAndBigInt }
