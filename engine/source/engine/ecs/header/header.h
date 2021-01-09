#pragma once
#include "../EntityType.h"
#include "../BaseComponentSystem.h"
#include "../BaseComponent.h"
#include "../ComponentDecorator.h"
#include "../GameWorld.h"
#include "../EntityManager.h"
#include "../EntityDecorator.h"

#include "../components/ActiveCom.h"
#include "../components/IDNameCom.h"
#include "../components/ParentCom.h"
#include "../components/ChildrenCom.h"
#include "../components/ParticleCom.h"
#include "../components/PerspectiveCameraCom.h"
#include "../components/LightCom.h"

#include "../components/3d/Body3DCom.h"
#include "../components/3d/Scene3DCom.h"
#include "../components/3d/Transform3DCom.h"
#include "../components/3d/Animation3DCom.h"
#include "../components/3d/Particle3DCom.h"

//#include "../components/2d/Body2DCom.h"
//#include "../components/2d/Sprite2DCom.h"
//#include "../components/2d/Animation3DCom.h"
//#include "../components/2d/AttachedMovement2DCom.h"
//#include "../components/2d/Transform2DCom.h"

#include "../component-systems/Body3DComSys.h"
#include "../component-systems/Transform3DComSys.h"
#include "../component-systems/PerspectiveCameraComSys.h"
#include "../component-systems/Scene3DComSys.h"
#include "../component-systems/Animation3DComSys.h"
#include "../component-systems/Particle3DComSys.h"

#include "../component-systems/misc/EntityGCComSys.h"

#include "../object-factory/ObjectFactory.h"