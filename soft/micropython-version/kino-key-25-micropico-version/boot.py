import usb_midi

# USBがPCに認識される（列挙される）前に、デバイス情報を書き換える
# 第1引数: VID (Vendor ID) - 任意（ここではテスト用の 0x1234）
# 第2引数: PID (Product ID) - 任意（ここではテスト用の 0x5678）
# 第3引数: メーカー名 (Manufacturer)
# 第4引数: 製品名 (Product)
# 第5引数: シリアルナンバー (Serial Number) - オプション
usb_midi.set_info(0x2E8A, 0x10F5, "Kinoshita Laboratory", "Kino-Key25 Micropython version", "kinokey25mpv")
