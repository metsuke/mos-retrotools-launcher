import tkinter as tk
from tkinter import messagebox
import subprocess
import platform
import os
from PIL import Image, ImageTk
import math

# Function to load or create Wine configuration for an app
def load_wine_config(subfolder):
    config_dir = os.path.join(os.path.dirname(__file__), 'assets', 'wine-config')
    config_path = os.path.join(config_dir, f"{subfolder}.cfg")
    
    # Ensure config directory exists
    os.makedirs(config_dir, exist_ok=True)
    
    if not os.path.exists(config_path):
        try:
            with open(config_path, 'w') as f:
                f.write("# Default Wine configuration for {}\n".format(subfolder))
                f.write("# Add Wine parameters here, one per line\n")
            print(f"Created default Wine config for {subfolder} at {config_path}")
            return []
        except Exception as e:
            messagebox.showwarning("Warning", f"Failed to create {subfolder}.cfg: {str(e)}. Using default Wine settings.")
            print(f"Error creating config: {str(e)}")
            return []
    
    try:
        with open(config_path, 'r') as f:
            #config = [line.strip() for line in f if line.strip() and not line.strip().startswith('#')]
            config = [os.path.expandvars(line.strip()) for line in f if line.strip() and not line.strip().startswith('#')]
        print(f"Loaded Wine config for {subfolder}: {config}")
        return config
    except Exception as e:
        messagebox.showwarning("Warning", f"Failed to read {subfolder}.cfg: {str(e)}. Using default Wine settings.")
        print(f"Error reading config: {str(e)}")
        return []

# Function to launch a program based on OS
def launch_program(exe_name, subfolder):
    current_os = platform.system().lower()
    exe_path = os.path.join(os.path.dirname(__file__), 'assets', 'bin', subfolder, exe_name)
    print(f"Attempting to launch: {exe_path} on {current_os}")
    if not os.path.exists(exe_path):
        messagebox.showerror("Error", f"File {exe_name} not found at {exe_path}")
        print(f"File not found: {exe_path}")
        return
    
    try:
        if current_os == 'windows':
            print(f"Launching on Windows: {exe_path}")
            subprocess.Popen([exe_path], shell=False)
        else:
            wine_args = ['wine']
            config_args = load_wine_config(subfolder)
            # Separate environment variables from command-line args
            env = os.environ.copy()
            for arg in config_args:
                if '=' in arg:
                    key, value = arg.split('=', 1)
                    env[key] = value
                else:
                    wine_args.append(arg)
            wine_args.append(exe_path)
            print(f"Launching with Wine: {wine_args} with env: {{k: v for k, v in env.items() if k in ['WINEDEBUG', 'WINEPREFIX', 'WINEARCH']}}")
            subprocess.Popen(wine_args, env=env, shell=False)
    except Exception as e:
        messagebox.showerror("Error", f"Failed to launch {exe_name}: {str(e)}")
        print(f"Launch error: {str(e)}")

# Global list of buttons
buttons = []

# List to retain PhotoImage objects
photo_references = []

# Function to add a button
def add_button(button_info):
    buttons.append(button_info)
    rebuild_gui()

# Function to remove a button by name
def remove_button(name):
    global buttons
    buttons = [b for b in buttons if b['name'] != name]
    rebuild_gui()

# Function to rebuild the GUI dynamically
def rebuild_gui():
    global photo_references
    # Clear existing widgets
    for widget in frame.winfo_children():
        widget.destroy()
    photo_references = []  # Clear old photo references

    n = len(buttons)
    if n == 0:
        root.geometry("200x100")
        return

    # Calculate grid side for square-ish layout
    side = math.ceil(math.sqrt(n))

    # Image directory
    img_dir = os.path.join(os.path.dirname(__file__), 'assets', 'img', 'buttons')
    print(f"Image directory: {img_dir}")

    # Load sizes
    sizes = []
    for b in buttons:
        path = os.path.join(img_dir, b['image_file'])
        print(f"Checking image: {path}")
        if not os.path.exists(path):
            messagebox.showwarning("Warning", f"Image file {b['image_file']} not found at {path}. Using fallback.")
            print(f"Image not found: {path}")
            sizes.append((100, 100))
            continue
        try:
            img = Image.open(path)
            sizes.append((img.width, img.height))
        except Exception as e:
            messagebox.showwarning("Warning", f"Failed to load {b['image_file']}: {str(e)}. Using fallback.")
            print(f"Image load error: {str(e)}")
            sizes.append((100, 100))

    # Load placeholder image size
    placeholder_path = os.path.join(img_dir, 'placeholder.png')
    print(f"Checking placeholder image: {placeholder_path}")
    if os.path.exists(placeholder_path):
        try:
            placeholder_img = Image.open(placeholder_path)
            sizes.append((placeholder_img.width, placeholder_img.height))
        except Exception as e:
            messagebox.showwarning("Warning", f"Failed to load placeholder.png: {str(e)}. Using fallback.")
            print(f"Placeholder image load error: {str(e)}")
            sizes.append((100, 100))
    else:
        print(f"Placeholder image not found: {placeholder_path}")
        sizes.append((100, 100))

    if not sizes:
        return

    # Calculate max width and max aspect ratio
    max_w = max(w for w, h in sizes)
    max_aspect = max(h / w for w, h in sizes)

    # Padding and sizes
    padding_per_cell = 20
    label_height = 30
    outer_padding_x = 20
    outer_padding_y = 40
    border_thickness = 8
    corner_radius = 10
    label_padding = 10  # Additional padding between button and label

    # Screen limits
    screen_width = root.winfo_screenwidth()
    screen_height = root.winfo_screenheight()
    max_window_w = screen_width // 3
    max_window_h = screen_height // 2

    # Tentative sizes
    tentative_button_w = max_w
    tentative_max_h = max_aspect * tentative_button_w
    cell_w = tentative_button_w + padding_per_cell + 2 * border_thickness
    cell_h = tentative_max_h + label_height + padding_per_cell + 2 * border_thickness + label_padding
    window_w = side * cell_w + outer_padding_x
    window_h = side * cell_h + outer_padding_y

    # Scale if necessary
    scale = min(max_window_w / window_w, max_window_h / window_h, 1.0)
    button_w = tentative_button_w * scale
    max_h = tentative_max_h * scale

    # Final window sizes
    cell_w = button_w + padding_per_cell + 2 * border_thickness
    cell_h = max_h + label_height + padding_per_cell + 2 * border_thickness + label_padding
    window_w = side * cell_w + outer_padding_x
    window_h = side * cell_h + outer_padding_y
    root.geometry(f"{int(window_w)}x{int(window_h)}")

    # Configure grid for uniform cells
    for i in range(side):
        frame.rowconfigure(i, weight=1, uniform="row")
        frame.columnconfigure(i, weight=1, uniform="col")

    # Create resized photoimages
    photoimages = {}
    for b in buttons:
        path = os.path.join(img_dir, b['image_file'])
        try:
            img = Image.open(path)
            new_w = int(button_w)
            new_h = int((img.height / img.width) * button_w)
            img_resized = img.resize((new_w, new_h), Image.LANCZOS)
            photo = ImageTk.PhotoImage(img_resized)
            photoimages[b['name']] = photo
            photo_references.append(photo)
            print(f"Loaded image for {b['name']}: {path}")
        except Exception as e:
            messagebox.showwarning("Warning", f"Failed to load {b['image_file']}: {str(e)}. Using placeholder.")
            print(f"Image load error for {b['name']}: {str(e)}")
            img = Image.new('RGB', (int(button_w), int(max_h)), color='gray')
            photo = ImageTk.PhotoImage(img)
            photoimages[b['name']] = photo
            photo_references.append(photo)

    # Load placeholder image
    try:
        placeholder_img = Image.open(placeholder_path)
        new_w = int(button_w)
        new_h = int((placeholder_img.height / placeholder_img.width) * button_w)
        placeholder_img_resized = placeholder_img.resize((new_w, new_h), Image.LANCZOS)
        placeholder_photo = ImageTk.PhotoImage(placeholder_img_resized)
        photo_references.append(placeholder_photo)
        print(f"Loaded placeholder image: {placeholder_path}")
    except Exception as e:
        messagebox.showwarning("Warning", f"Failed to load placeholder.png: {str(e)}. Using gray fallback.")
        print(f"Placeholder image load error: {str(e)}")
        placeholder_img = Image.new('RGB', (int(button_w), int(max_h)), color='#808080')
        placeholder_photo = ImageTk.PhotoImage(placeholder_img)
        photo_references.append(placeholder_photo)

    # Sort buttons alphabetically
    sorted_buttons = sorted(buttons, key=lambda b: b['name'])

    # Place buttons and placeholders
    idx = 0
    for row in range(side):
        for col in range(side):
            if idx < n:
                b = sorted_buttons[idx]
                bg_color = "black"
                border_color = "red" if b['status'] == 'wip' else "black"
                
                subframe = tk.Frame(frame, bg=bg_color)
                subframe.grid(row=row, column=col, sticky="nsew", padx=10, pady=10)
                
                button_frame = tk.Frame(subframe, bg=bg_color)
                button_frame.pack(pady=border_thickness)
                
                canvas = tk.Canvas(button_frame, bg=bg_color, highlightthickness=0, width=button_w + 2 * border_thickness, height=max_h + 2 * border_thickness)
                canvas.pack()
                canvas.create_rounded_rectangle = lambda x1, y1, x2, y2, r, **kwargs: canvas.create_polygon(
                    x1+r, y1, x2-r, y1, x2, y1+r, x2, y2-r, x2-r, y2, x1+r, y2, x1, y2-r, x1, y1+r,
                    smooth=True, **kwargs
                )
                canvas.create_rounded_rectangle(
                    border_thickness, border_thickness,
                    button_w + border_thickness, max_h + border_thickness,
                    corner_radius, outline=border_color, width=border_thickness
                )

                if b['status'] == 'functional':
                    cmd = lambda name=b['name'], exe=b['exe'], sub=b['subfolder']: (print(f"Clicked {name}"), launch_program(exe, sub))
                    print(f"Binding functional button: {b['name']}")
                else:
                    cmd = lambda name=b['name']: (print(f"Clicked WIP: {name}"), messagebox.showinfo("WIP", f"{name}: Aun trabajando para arrancarlo con wine"))
                    print(f"Binding WIP button: {b['name']}")

                button = tk.Button(button_frame, image=photoimages[b['name']], command=cmd, cursor="hand2", borderwidth=0, relief="flat", bg=bg_color)
                button.place(x=border_thickness, y=border_thickness, width=button_w, height=max_h)

                label = tk.Label(subframe, text=b['name'], font=("Arial", 12, "bold"), bg=bg_color, fg="white")
                label.pack(pady=label_padding)
                idx += 1
            else:
                subframe = tk.Frame(frame, bg='black')
                subframe.grid(row=row, column=col, sticky="nsew", padx=10, pady=10)
                button_frame = tk.Frame(subframe, bg='black')
                button_frame.pack(pady=border_thickness)
                canvas = tk.Canvas(button_frame, bg='black', highlightthickness=0, width=button_w + 2 * border_thickness, height=max_h + 2 * border_thickness)
                canvas.pack()
                canvas.create_rounded_rectangle = lambda x1, y1, x2, y2, r, **kwargs: canvas.create_polygon(
                    x1+r, y1, x2-r, y1, x2, y1+r, x2, y2-r, x2-r, y2, x1+r, y2, x1, y2-r, x1, y1+r,
                    smooth=True, **kwargs
                )
                canvas.create_rounded_rectangle(
                    border_thickness, border_thickness,
                    button_w + border_thickness, max_h + border_thickness,
                    corner_radius, outline='#808080', width=border_thickness
                )
                placeholder_button = tk.Label(button_frame, image=placeholder_photo, bg='black', borderwidth=0)
                placeholder_button.place(x=border_thickness, y=border_thickness, width=button_w, height=max_h)
                placeholder_label = tk.Label(subframe, text="-", font=("Arial", 12, "bold"), bg='black', fg="white")
                placeholder_label.pack(pady=label_padding)

# Create the main window
root = tk.Tk()
root.title("MetsuOS RetroLauncher v0.0.4 (Alpha")

# Make window non-resizable
root.resizable(False, False)

# Create a frame to hold buttons
frame = tk.Frame(root)
frame.pack(expand=True, fill='both', padx=10, pady=10)

# Initial buttons (add them)
add_button({
    'name': "AYFX-EDIT",
    'image_file': "ayfxedit.png",
    'exe': "ayfxedit.exe",
    'subfolder': "ayfxedit",
    'status': "functional"
})
add_button({
    'name': "RETRO-X",
    'image_file': "retro-x.jpg",
    'exe': "Retro-X.exe",
    'subfolder': "retro-x",
    'status': "functional"
})
add_button({
    'name': "VORTEX TRACKER II",
    'image_file': "vortex-tracker-ii.png",
    'exe': "VT.exe",
    'subfolder': "vortex-tracker-ii",
    'status': "wip"
})
add_button({
    'name': "ZX-PAINTBRUSH",
    'image_file': "zx-paintbrush.png",
    'exe': "ZXPaintbrush.exe",
    'subfolder': "zx-paintbrush",
    'status': "functional"
})

# Run the GUI loop
root.mainloop()