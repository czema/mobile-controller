export interface Config {
  version: 2;
  rooms: Room[];
  devices: Device[];
}

export interface Room {
  id: string;
  name: string;
  color?: string;
}

export interface Device {
  id: string;
  roomId: string;
  name: string;
  type: 'ir' | 'rf';
  slot: number;
  x: number;
  y: number;
  iconName: string;
  lastSent: string | null;
}
