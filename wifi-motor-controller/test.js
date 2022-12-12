async function status() {
  const url = 'http://192.168.68.66/api/status'
  const response = await fetch(url, {
    method: 'POST',
    headers: {
      accept: 'application/json',
      'content-type': 'application/json'
    },
    body: JSON.stringify({ test: true })
  })
  const data = await response.json()
  console.log('DATA', data)
}

async function move({ degrees=90, speed, acceleration, maxSpeed, resolution }) {
  const url = 'http://192.168.68.66/api/move'
  const response = await fetch(url, {
    method: 'POST',
    headers: {
      accept: 'application/json',
      'content-type': 'application/json'
    },
    body: JSON.stringify({ degrees, speed, acceleration, maxSpeed, resolution })
  })
  const data = await response.json()
  console.log('DATA', data)
}

async function config({ speed, acceleration, maxSpeed, resolution }) {
  const url = 'http://192.168.68.66/api/config'
  const response = await fetch(url, {
    method: 'POST',
    headers: {
      accept: 'application/json',
      'content-type': 'application/json'
    },
    body: JSON.stringify({ speed, acceleration, maxSpeed, resolution })
  })
  const data = await response.json()
  console.log('DATA', data)
}

async function mode(mode='normal') {
  const url = 'http://192.168.68.66/api/move'
  const response = await fetch(url, {
    method: 'POST',
    headers: {
      accept: 'application/json',
      'content-type': 'application/json'
    },
    body: JSON.stringify({ mode })
  })
  const data = await response.json()
  console.log('DATA', data)
}
