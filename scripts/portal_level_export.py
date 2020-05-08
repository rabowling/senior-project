import bpy
# ExportHelper is a helper class, defines filename and
# invoke() function which calls the file selector.
from bpy_extras.io_utils import ExportHelper, axis_conversion
from bpy.props import StringProperty, BoolProperty, EnumProperty
from bpy.types import Operator

bl_info = {
    "name": "Portal Level Exporter",
    "description": "Export Portal levels for senior project.",
    "author": "Connor Virostek, Reed Bowling",
    "version": (1, 0),
    "blender": (2, 80, 0),
    "location": "File > Import-Export",
    "warning": "", # used for warning icon and text in addons panel
    "wiki_url": "",
    "tracker_url": "",
    "support": "COMMUNITY",
    "category": "Import-Export"
}

def write_some_data(context, filepath, use_some_setting):
    print("running write_some_data...")
    
    lines = []
    for collection in bpy.data.collections:
        for obj in collection.objects:
            serial = serialize_object(collection.name, obj)
            if serial is not None:
                lines.append(serial)
    
    f = open(filepath, 'w', encoding='utf-8')
    
    f.write('\n'.join(lines))
    
    f.close()

    return {'FINISHED'}

def serialize_object(collection, obj):
    conversion_mat = axis_conversion(from_forward='-Y', from_up='Z', to_forward='Z', to_up='Y').to_4x4()
    m = conversion_mat @ obj.matrix_world
    if collection == 'Player':
        return 'player {t.x} {t.y} {t.z}'.format(t = m.to_translation())
    elif collection == 'Walls':
        return ('wall {t.x} {t.y} {t.z} {s.x} {s.y} {s.z} {r.x} {r.y} {r.z} {r.w}'
            .format(t = m.to_translation(), s = m.to_scale(), r = m.to_quaternion()))
    elif collection == 'Buttons':
        return ('button {t.x} {t.y} {t.z}'
            .format(t = m.to_translation()))
    elif collection == 'Boxes':
        return ('box {t.x} {t.y} {t.z} {s.x} {s.y} {s.z} {r.x} {r.y} {r.z} {r.w}'
            .format(t = m.to_translation(), s = m.to_scale(), r = m.to_quaternion()))
    elif collection == 'Doors':
        pass
    elif collection == 'Portals':
        return ('portal {id} {link} {t.x} {t.y} {t.z} {s.x} {s.y} {s.z} {r.w} {r.x} {r.y} {r.z}'
            .format(id = obj["id"], link = obj["link"], t = m.to_translation(), s = m.to_scale(), r = m.to_quaternion()))
    elif collection == 'Lights':
        return ('light {t.x} {t.y} {t.z} {c.r} {c.g} {c.b} {id}'
            .format(id = obj["id"], t = m.to_translation(), c = obj.data.color))
    elif collection == "LightSwitches":
        return ('lightswitch {light} {t.x} {t.y} {t.z}'
            .format(light = obj["light"], t = m.to_translation()))
    else:
        return None

    print('Error: could not write object ' + obj.name)
    return None

class ExportPortalLevel(Operator, ExportHelper):
    """Export scene as a Portal level."""
    bl_idname = "export_test.some_data"  # important since its how bpy.ops.import_test.some_data is constructed
    bl_label = "Export Level"

    # ExportHelper mixin class uses this
    filename_ext = ".txt"

    filter_glob: StringProperty(
        default="*.txt",
        options={'HIDDEN'},
        maxlen=255,  # Max internal buffer length, longer would be clamped.
    )

    # List of operator properties, the attributes will be assigned
    # to the class instance from the operator settings before calling.
    use_setting: BoolProperty(
        name="Example Boolean",
        description="Example Tooltip",
        default=True,
    )

    type: EnumProperty(
        name="Example Enum",
        description="Choose between two items",
        items=(
            ('OPT_A', "First Option", "Description one"),
            ('OPT_B', "Second Option", "Description two"),
        ),
        default='OPT_A',
    )

    def execute(self, context):
        return write_some_data(context, self.filepath, self.use_setting)


# Only needed if you want to add into a dynamic menu
def menu_func_export(self, context):
    self.layout.operator(ExportPortalLevel.bl_idname, text="Portal Level (.txt)")


def register():
    bpy.utils.register_class(ExportPortalLevel)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(ExportPortalLevel)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    register()
    
    # test call
    bpy.ops.export_test.some_data('INVOKE_DEFAULT')
