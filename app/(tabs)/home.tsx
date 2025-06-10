import React, { useState, useEffect } from 'react';
import { View, Text, TouchableOpacity, ScrollView, Dimensions, Alert, ActivityIndicator } from 'react-native';
import { useRouter } from 'expo-router';
import { Power, Sun, Moon, Timer } from 'lucide-react-native';
import { controlLight } from '@/api/controlLight';
import { getLightStatus } from '@/api/lightList'; 
import { useAuth } from '@/context/AuthProvider';

const { width, height } = Dimensions.get('window');

export default function HomeScreen() {
  const router = useRouter();
  const { user, token } = useAuth(); 

  const [lightStates, setLightStates] = useState<any>({});
  const [loading, setLoading] = useState<boolean>(false);
  const [selectedLightId, setSelectedLightId] = useState<string | null>(null);

  const fetchLightStatus = async () => {
    try {
      if (!user?.email) {
        Alert.alert('Error', 'User email not found.');
        return;
      }
      setLoading(true);
      const result = await getLightStatus(undefined, token ?? undefined); // ✅ No token argument
      setLightStates(result);
      setLoading(false);
    } catch (error) {
      Alert.alert('Error', 'Failed to fetch light status.');
      setLoading(false);
    }
  };

  const toggleLight = async (lightId: string, currentStatus: string) => {
    try {
      if (!user?.email) {
        Alert.alert('Error', 'User email not found.');
        return;
      }

      const newStatus = currentStatus === 'ON' ? 'OFF' : 'ON';
      await controlLight(newStatus, lightId, token ?? undefined); 

      // After toggling, refresh status:
      await fetchLightStatus();
    } catch (error) {
      Alert.alert('Error', `Failed to toggle light ${lightId}.`);
    }
  };

  const handleGetStatus = async (lightId: string) => {
    try {
      if (!user?.email) {
        Alert.alert('Error', 'User email not found.');
        return;
      }

      const result = await getLightStatus([lightId], token ?? undefined); // ✅ No token argument
      console.log(`✅ Status for ${lightId}:`, result);

      Alert.alert(`Light ${lightId} Status`, JSON.stringify(result[lightId], null, 2));
    } catch (error) {
      Alert.alert('Error', `Failed to get status for light ${lightId}.`);
    }
  };

  const handleLongPress = (id: string) => {
    setSelectedLightId(id);
  };

  const closeDetails = () => {
    setSelectedLightId(null);
  };

  useEffect(() => {
  // Initial fetch
    fetchLightStatus();

    const intervalId = setInterval(() => {
      fetchLightStatus();
    }, 300000);

  // Clean up interval on unmount
    return () => clearInterval(intervalId);
  }, []);

  return (
    <View className="flex-1 bg-[#121212]">
      {/* Header */}
      <View className="bg-[#2a2b33] py-6 px-4 items-center justify-center">
        <Text className="text-white text-5xl font-bold">Home</Text>
      </View>

      <TouchableOpacity className="px-4 py-3" onPress={fetchLightStatus}>
        <Text className="text-white text-lg font-semibold">↻ Refresh Lights</Text>
      </TouchableOpacity>

      {loading ? (
        <ActivityIndicator size="large" color="#00ff00" className="mt-10" />
      ) : (
        <ScrollView className="px-4" contentContainerStyle={{ paddingBottom: 100 }}>
          {Object.keys(lightStates).map((lightId) => {
            const lightData = lightStates[lightId];
            const isOn = lightData?.status === 'ON';
            const isSelected = selectedLightId === lightId;

            return (
              <TouchableOpacity
                key={lightId}
                onPress={() => toggleLight(lightId, lightData?.status)}
                onLongPress={() => handleLongPress(lightId)}
                className={`px-5 py-4 mb-3 rounded-2xl ${
                  isOn ? 'bg-[#0a3d62]' : 'bg-[#2f2f2f]'
                }`}
              >
                <View className="flex-row justify-between items-center">
                  <View>
                    <Text className="text-white text-lg font-bold">Light ID: {lightId}</Text>
                    <Text className="text-gray-300">Status: {lightData?.status || 'Unknown'}</Text>
                  </View>
                  <Power color={isOn ? '#00ff00' : '#ff0000'} size={32} />
                </View>

                {isSelected && (
                  <View className="mt-4">
                    <View className="flex-row justify-between items-center mb-3">
                      <Text className="text-white text-lg font-semibold">Routines</Text>
                      <TouchableOpacity onPress={closeDetails}>
                        <Text className="text-white font-semibold">Back</Text>
                      </TouchableOpacity>
                    </View>
                    <TouchableOpacity className="bg-[#0a3d62] p-4 mb-2 rounded-xl flex-row items-center">
                      <Sun color="white" />
                      <Text className="text-white ml-2">Wake up</Text>
                    </TouchableOpacity>
                    <TouchableOpacity className="bg-[#0a3d62] p-4 mb-2 rounded-xl flex-row items-center">
                      <Moon color="white" />
                      <Text className="text-white ml-2">Go to sleep</Text>
                    </TouchableOpacity>
                    <TouchableOpacity
                      className="bg-[#0a3d62] p-4 mb-2 rounded-xl flex-row items-center"
                      onPress={() => handleGetStatus(lightId)}
                    >
                      <Timer color="white" />
                      <Text className="text-white ml-2">Check Status</Text>
                    </TouchableOpacity>
                  </View>
                )}
              </TouchableOpacity>
            );
          })}
        </ScrollView>
      )}
    </View>
  );
}
