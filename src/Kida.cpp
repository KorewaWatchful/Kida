#include "Kida.h"
#include <miniupnpc.h>
#include <upnpcommands.h>
#include <upnperrors.h>

#include <spdlog/spdlog.h>
#ifdef WIN32
#include <WinSock2.h>
#endif

#include <optional>
#include <functional>
#include <vector>

using conn_flow_fn = std::function<std::optional<int>()>;

std::optional<int> upnp_flow() {
	spdlog::debug("Attempting UPnP flow...");

#ifdef WIN32
	WSADATA wsa_data;
	int winsock_res = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (winsock_res != NO_ERROR) {
		spdlog::debug("Winsocket startup failed with error: {}", winsock_res);
		return std::nullopt;
	}
#endif

	int error = 0;
	UPNPDev* devlist = upnpDiscover(2000, NULL, NULL, UPNP_LOCAL_PORT_ANY, 0, 2, &error);
	if (!devlist) {
		spdlog::debug("No UPnP devices found. Error: {}", error);
		return std::nullopt;
	}

	UPNPUrls urls;
	IGDdatas data;
	char lanaddr[64], wanaddr[64];

	int igd_status = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr), wanaddr, sizeof(wanaddr));
	freeUPNPDevlist(devlist);

	if (igd_status <= 0) {
		spdlog::debug("No valid IGD found!");
		return std::nullopt;
	}

	spdlog::debug("LAN IP: {}, WAN IP: {}", lanaddr, wanaddr);

	char external_ip[40];
	if (UPNP_GetExternalIPAddress(urls.controlURL, data.first.servicetype, external_ip) != UPNPCOMMAND_SUCCESS) {
		spdlog::debug("Failed to get external IP.");
		return std::nullopt;
	}

	spdlog::debug("External IP: {}", external_ip);

	const char* port = "8888";
	int result = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, port, port, lanaddr, "Kida", "UDP", nullptr, "0");

	if (result != UPNPCOMMAND_SUCCESS) {
		spdlog::debug("AddPortMapping failed: {}", strupnperror(result));
		return std::nullopt;
	}

	FreeUPNPUrls(&urls);
#ifdef WIN32
	WSACleanup();
#endif

	spdlog::debug("Port {} mapped successfully!", port);
	return 1;
}

std::optional<int> udp_hp_flow() {
	spdlog::debug("Attempting UDP Hole Punch...");

	bool success = true;
	if (!success) {
		spdlog::debug("UDP Hole Punch has failed!");
		return std::nullopt;
	}
	return 1;
}

std::optional<int> establish_conn(const std::vector<conn_flow_fn>& methods) {
	for (const auto& method : methods) {
		try {
			if (auto result = method()) {
				return result;
			}
		}
		catch (const std::exception& e) {
			spdlog::error("Exception encountered during connecion attempt: {}", e.what());
			continue;
		}
	}
	return std::nullopt;
}

int main()
{
	spdlog::set_level(spdlog::level::debug);
	std::vector<conn_flow_fn> conn_methods = { upnp_flow, udp_hp_flow };

	if (auto result = establish_conn(conn_methods)) {
		spdlog::debug("Success!");
		return 0;
	}
	else {
		spdlog::error("All connection flows exhausted with no success.");
		return 1;
	}
}
