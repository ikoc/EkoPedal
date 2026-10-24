"""STL -> PNG offscreen render via VTK (cadquery dependency)."""
import sys, vtk

def render(stl_paths, out_png, colors=None, cam=(1.4, -1.0, 0.9), size=(1400, 1000), zoom=1.0,
           light_position=(200, -300, 400), fill_light_position=(-300, 200, 100), ambient=0.0):
    ren = vtk.vtkRenderer(); ren.SetBackground(0.96, 0.96, 0.97)
    ren.SetUseFXAA(True)
    default_cols = [(0.16,0.17,0.19),(0.75,0.76,0.78),(0.16,0.17,0.19),(0.55,0.57,0.6),(0.85,0.6,0.2)]
    for i, p in enumerate(stl_paths):
        r = vtk.vtkSTLReader(); r.SetFileName(p); r.Update()
        n = vtk.vtkPolyDataNormals(); n.SetInputConnection(r.GetOutputPort()); n.SetFeatureAngle(35); n.Update()
        m = vtk.vtkPolyDataMapper(); m.SetInputConnection(n.GetOutputPort())
        a = vtk.vtkActor(); a.SetMapper(m)
        c = (colors or default_cols)[i % len(colors or default_cols)]
        a.GetProperty().SetColor(*c); a.GetProperty().SetSpecular(0.35); a.GetProperty().SetSpecularPower(30)
        a.GetProperty().SetAmbient(ambient)
        a.GetProperty().SetInterpolationToPhong()
        ren.AddActor(a)
    rw = vtk.vtkRenderWindow(); rw.SetOffScreenRendering(1); rw.SetSize(*size); rw.AddRenderer(ren)
    rw.SetMultiSamples(8)
    light = vtk.vtkLight(); light.SetPosition(*light_position); light.SetIntensity(1.0); ren.AddLight(light)
    l2 = vtk.vtkLight(); l2.SetPosition(*fill_light_position); l2.SetIntensity(0.5); ren.AddLight(l2)
    ren.ResetCamera()
    c = ren.GetActiveCamera(); fp = c.GetFocalPoint()
    d = c.GetDistance()
    c.SetPosition(fp[0]+cam[0]*d, fp[1]+cam[1]*d, fp[2]+cam[2]*d); c.SetViewUp(0,0,1)
    ren.ResetCamera(); c.Zoom(zoom)
    rw.Render()
    w2i = vtk.vtkWindowToImageFilter(); w2i.SetInput(rw); w2i.Update()
    wr = vtk.vtkPNGWriter(); wr.SetFileName(out_png); wr.SetInputConnection(w2i.GetOutputPort()); wr.Write()
    print("wrote", out_png)

if __name__ == "__main__":
    render(sys.argv[2:], sys.argv[1])

def grid(stl_paths, out_png, cams, colors=None, size=(900, 700)):
    """Birden fazla acidan render alip tek PNG'de yan yana birlestir."""
    import os, tempfile
    from PIL import Image
    tiles = []
    for i, cam in enumerate(cams):
        p = os.path.join(tempfile.gettempdir(), f"_tile{i}.png")
        render(stl_paths, p, colors=colors, cam=cam, size=size)
        tiles.append(Image.open(p))
    cols = 2 if len(tiles) > 1 else 1
    rows = (len(tiles) + cols - 1) // cols
    W, H = tiles[0].size
    img = Image.new("RGB", (W * cols, H * rows), (245, 245, 247))
    for i, t in enumerate(tiles):
        img.paste(t, ((i % cols) * W, (i // cols) * H))
    img.save(out_png); print("wrote", out_png)
