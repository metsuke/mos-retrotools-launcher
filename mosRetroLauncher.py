import tkinter as tk
from tkinter import messagebox
import subprocess
import platform
import os
from PIL import Image, ImageTk

# Function to launch a program based on OS
def launch_program(exe_name, subfolder):
    current_os = platform.system().lower()
    exe_path = os.path.join(os.path.dirname(__file__), 'assets', 'bin', subfolder, exe_name)
    #messagebox.showinfo ("Ruta exe",f"{exe_path}")
    if not os.path.exists(exe_path):
        messagebox.showerror("Error", f"File {exe_name} not found in assets/bin/{subfolder} directory.")
        return
    
    try:
        if current_os == 'windows':
            subprocess.Popen([exe_path])
        else:
            # For Linux or Mac, use wine
            subprocess.Popen(['wine', exe_path])
    except Exception as e:
        messagebox.showerror("Error", f"Failed to launch {exe_name}: {str(e)}")

# Create the main window
root = tk.Tk()
root.title("MetsuOS RetroLauncher")

# Make window non-resizable
root.resizable(False, False)

# Get screen resolution
screen_width = root.winfo_screenwidth()
screen_height = root.winfo_screenheight()

# Load images to determine their size
try:
    vortex_img = Image.open(os.path.join(os.path.dirname(__file__), 'assets', 'img', 'buttons', 'vortex-tracker-ii.png'))
    zx_img = Image.open(os.path.join(os.path.dirname(__file__), 'assets', 'img', 'buttons', 'zx-paintbrush.png'))
except Exception as e:
    messagebox.showerror("Error", f"Failed to load images: {str(e)}\nMake sure PNG files are in assets/img/buttons directory.")
    root.quit()

# Calculate window size based on image sizes
vortex_width, vortex_height = vortex_img.size
zx_width, zx_height = zx_img.size

# Use the maximum width and height to ensure both buttons fit
button_width = max(vortex_width, zx_width)
button_height = max(vortex_height, zx_height)

# Calculate window size: twice the button width (for two buttons) + padding
padding_x = 40  # Total horizontal padding (20 per side)
padding_y = 60  # Total vertical padding (20 top, 20 bottom, 20 for label)
label_height = 30  # Approximate height for the label (Arial 12 bold)
window_width = 2 * button_width + padding_x
window_height = button_height + label_height + padding_y

# Restrict window size to 25% of screen width and 50% of screen height if images are too large
max_window_width = screen_width // 4
max_window_height = screen_height // 2
if window_width > max_window_width or window_height > max_window_height:
    # Scale down images to fit within max window size
    scale_factor = min(max_window_width / window_width, max_window_height / window_height)
    button_width = int(button_width * scale_factor)
    button_height = int(button_height * scale_factor)
    window_width = 2 * button_width + padding_x
    window_height = button_height + label_height + padding_y

# Set window size
root.geometry(f"{window_width}x{window_height}")

# Create a frame to hold buttons
frame = tk.Frame(root)
frame.pack(expand=True, fill='both', padx=10, pady=10)

# Load and resize images to fit 50% of window width
def resize_image(image_path, width):
    img = Image.open(image_path)
    aspect_ratio = img.height / img.width
    new_height = int(width * aspect_ratio)
    img_resized = img.resize((width, new_height), Image.LANCZOS)
    return ImageTk.PhotoImage(img_resized)

try:
    vortex_image = resize_image(os.path.join(os.path.dirname(__file__), 'assets', 'img', 'buttons', 'vortex-tracker-ii.png'), button_width)
    zx_image = resize_image(os.path.join(os.path.dirname(__file__), 'assets', 'img', 'buttons', 'zx-paintbrush.png'), button_width)
except Exception as e:
    messagebox.showerror("Error", f"Failed to load images: {str(e)}")
    root.quit()

# Button for Vortex Tracker II (alphabetically first, so on the left)
vortex_frame = tk.Frame(frame)
vortex_frame.pack(side=tk.LEFT, padx=10)
#vortex_button = tk.Button(vortex_frame, image=vortex_image, command=lambda: launch_program('VT.exe', 'vortex-tracker-ii'), cursor="hand2")
vortex_button = tk.Button(vortex_frame, image=vortex_image, command=lambda: messagebox.showinfo ("WIP",f"Aun trabajando para arrancarlo con wine"))

vortex_button.pack()
vortex_label = tk.Label(vortex_frame, text="VORTEX TRACKER II", font=("Arial", 12, "bold"))
vortex_label.pack()

# Button for ZX-Paintbrush (alphabetically second, so on the right)
zx_frame = tk.Frame(frame)
zx_frame.pack(side=tk.RIGHT, padx=10)
zx_button = tk.Button(zx_frame, image=zx_image, command=lambda: launch_program('ZXPaintbrush.exe', 'zx-paintbrush'), cursor="hand2")
zx_button.pack()
zx_label = tk.Label(zx_frame, text="ZX-PAINTBRUSH", font=("Arial", 12, "bold"))
zx_label.pack()

# Run the GUI loop
root.mainloop()