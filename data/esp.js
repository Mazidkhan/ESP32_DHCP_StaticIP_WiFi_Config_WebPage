function showSection(section) {
    document.getElementById('networkSection').style.display = 'none';
    document.getElementById('calibrationSection').style.display = 'none';
    document.getElementById(section).style.display = 'block';
  }
  
  function showConnectWiFi() {
    document.getElementById('connectWiFiModal').style.display = 'block';
  }
  
  function showIPMode() {
    document.getElementById('ipModeModal').style.display = 'block';
  }
  
  function closeModal(modalId) {
    document.getElementById(modalId).style.display = 'none';
  }
  
  function toggleIPFields() {
    var ipMode = document.getElementById('ip_mode').value;
    document.getElementById('staticFields').style.display = (ipMode == 'static') ? 'block' : 'none';
    updateIPModeText(ipMode);
  }
  
  function updateIPModeText(ipMode) {
    var ipModeText = document.getElementById('ipModeText');
    ipModeText.innerText = 'IP Mode: ' + (ipMode == 'static' ? 'Static' : 'DHCP');
  }
  
function updateSensorData() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', '/sensorData', true);
    xhr.onreadystatechange = function() {
      if (xhr.readyState == 4 && xhr.status == 200) {
        var data = JSON.parse(xhr.responseText);
        document.getElementById('temperature').innerText = 'Temperature: ' + data.temperature + ' °C';
        document.getElementById('humidity').innerText = 'Humidity: ' + data.humidity + ' %';
      }
    };
    xhr.send();
}

setInterval(updateSensorData, 5000);
window.onload = updateSensorData;

function loadConnectedNetwork() {
  var xhr = new XMLHttpRequest();
  xhr.open('GET', '/connectedNetwork', true);  // Assuming '/connectedNetwork' returns the connected Wi-Fi SSID
  xhr.onreadystatechange = function() {
      if (xhr.readyState == 4 && xhr.status == 200) {
          var data = JSON.parse(xhr.responseText);
          var connectedNetworkElement = document.getElementById('connectedNetwork');
          if (data.SSID) {
            connectedNetworkElement.innerHTML = "SSID: " + data.SSID + "<br>" +
            "IP: " + data.IP_Address + "<br>" +
            "Mode: " + data.IP_Mode;
          } else {
              connectedNetworkElement.innerHTML = "Not connected to any network";
          }
      }
  };
  xhr.send();
}

function loadAvailableNetworks() {
  var xhr = new XMLHttpRequest();
  xhr.open('GET', '/availableNetworks', true);
  xhr.onreadystatechange = function() {
    if (xhr.readyState == 4 && xhr.status == 200) {
      // Log the response to check the data format
      console.log('Response received:', xhr.responseText);

      var data;
      try {
        // Try parsing the JSON response
        data = JSON.parse(xhr.responseText);
      } catch (e) {
        console.error('Error parsing JSON:', e);
        return;
      }

      var networksList = document.getElementById('availableNetworksList');
      networksList.innerHTML = '';  // Clear existing list

      if (data.networks && data.networks.length === 0) {
        networksList.innerHTML = '<li>No networks found</li>';
      } else if (data.networks) {
        // Loop through the available networks and add them to the list
        for (var i = 0; i < data.networks.length; i++) {
          var listItem = document.createElement('li');
          listItem.innerHTML = "SSID: " + data.networks[i].SSID + " (RSSI: " + data.networks[i].RSSI + " dBm)";
          networksList.appendChild(listItem);
        }
      } else {
        networksList.innerHTML = '<li>No networks data available</li>';
      }
    } else if (xhr.readyState == 4) {
      // Log an error if the request failed
      console.error('Error: Failed to load networks, status:', xhr.status);
    }
  };
  xhr.send();
}
window.onload = function() {
  loadAvailableNetworks();  // Load available networks
  loadConnectedNetwork();   // Load the connected network
};