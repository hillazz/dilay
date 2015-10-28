/* This file is part of Dilay
 * Copyright © 2015 Alexander Bau
 * Use and redistribute under the terms of the GNU General Public License
 */
#include <QCheckBox>
#include <QFrame>
#include <QMouseEvent>
#include <QRadioButton>
#include "cache.hpp"
#include "dimension.hpp"
#include "scene.hpp"
#include "sketch/mesh.hpp"
#include "sketch/node-intersection.hpp"
#include "sketch/path-intersection.hpp"
#include "state.hpp"
#include "tools.hpp"
#include "util.hpp"
#include "view/properties.hpp"
#include "view/tool-tip.hpp"
#include "view/util.hpp"

namespace {
  enum class Mode { DeleteMesh, DeleteNode, DeleteSpheres };

  int fromMode (Mode m) {
    switch (m) {
      case Mode::DeleteMesh:    return 0;
      case Mode::DeleteNode:    return 1;
      case Mode::DeleteSpheres: return 2;
      default:                  DILAY_IMPOSSIBLE;
    }
  }

  Mode toMode (int m) {
    switch (m) {
      case  0: return Mode::DeleteMesh;
      case  1: return Mode::DeleteNode;
      case  2: return Mode::DeleteSpheres;
      default: DILAY_IMPOSSIBLE;
    }
  }
}

struct ToolDeleteSketch::Impl {
  ToolDeleteSketch* self;
  Mode              mode;
  bool              deleteChildren;

  Impl (ToolDeleteSketch* s) 
    : self           (s)
    , mode           (toMode (s->cache ().get <int> ("mode", fromMode (Mode::DeleteNode))))
    , deleteChildren (s->cache ().get <bool> ("delete-children", false))
  {
    this->self->renderMirror (false);

    this->setupProperties ();
    this->setupToolTip    ();
  }

  void setupProperties () {
    ViewPropertiesPart& properties = this->self->properties ().body ();

    QCheckBox& deleteChildrenEdit = ViewUtil::checkBox ( QObject::tr ("Delete children")
                                                       , this->deleteChildren );
    ViewUtil::connect (deleteChildrenEdit, [this] (bool m) {
      this->deleteChildren = m;
      this->self->cache ().set ("delete-children", m);
    });
    deleteChildrenEdit.setEnabled (this->mode == Mode::DeleteNode);

    QCheckBox& mirrorEdit = ViewUtil::checkBox ( QObject::tr ("Mirror")
                                               , this->self->hasMirror () );
    ViewUtil::connect (mirrorEdit, [this] (bool m) {
      this->self->mirror (m);
    });
    mirrorEdit.setEnabled (this->mode != Mode::DeleteMesh);

    QRadioButton& deleteMeshEdit = ViewUtil::radioButton ( QObject::tr ("Delete mesh")
                                                         , this->mode == Mode::DeleteMesh );
    ViewUtil::connect (deleteMeshEdit, 
      [this, &deleteChildrenEdit, &mirrorEdit] (bool m)
    {
      this->mode = Mode::DeleteMesh;
      this->self->cache ().set ("mode", fromMode (this->mode));

      deleteChildrenEdit.setEnabled (!m);
      mirrorEdit        .setEnabled (!m);
    });

    QRadioButton& deleteNodeEdit = ViewUtil::radioButton ( QObject::tr ("Delete node")
                                                         , this->mode == Mode::DeleteNode );
    ViewUtil::connect (deleteNodeEdit, 
      [this, &deleteChildrenEdit, &mirrorEdit] (bool m)
    {
      this->mode = Mode::DeleteNode;
      this->self->cache ().set ("mode", fromMode (this->mode));

      deleteChildrenEdit.setEnabled (m);
      mirrorEdit        .setEnabled (m);
    });

    QRadioButton& deleteSpheresEdit = ViewUtil::radioButton ( QObject::tr ("Delete spheres")
                                                            , this->mode == Mode::DeleteSpheres );
    ViewUtil::connect (deleteSpheresEdit, 
      [this, &deleteChildrenEdit, &mirrorEdit] (bool m)
    {
      this->mode = Mode::DeleteSpheres;
      this->self->cache ().set ("mode", fromMode (this->mode));

      deleteChildrenEdit.setEnabled (!m);
      mirrorEdit        .setEnabled (m);
    });

    properties.add (deleteMeshEdit);
    properties.add (deleteNodeEdit);
    properties.add (deleteSpheresEdit);
    properties.add (ViewUtil::horizontalLine ());
    properties.add (deleteChildrenEdit);
    properties.add (mirrorEdit);
  }

  void setupToolTip () {
    ViewToolTip toolTip;
    toolTip.add (ViewToolTip::MouseEvent::Left, QObject::tr ("Delete selection"));
    this->self->showToolTip (toolTip);
  }

  ToolResponse runMouseReleaseEvent (const QMouseEvent& e) {
    if (e.button () == Qt::LeftButton) {
      switch (this->mode) {
        case Mode::DeleteMesh: {
          SketchMeshIntersection intersection;
          if (this->self->intersectsScene (e, intersection)) {
            this->self->snapshotSketchMeshes ();
            this->self->state ().scene ().deleteMesh (intersection.mesh ());
          }
          return ToolResponse::Redraw;
        }
        case Mode::DeleteNode: {
          SketchNodeIntersection intersection;
          if (this->self->intersectsScene (e, intersection)) {
            this->self->snapshotSketchMeshes ();
            intersection.mesh ().deleteNode ( intersection.node ()
                                            , this->deleteChildren
                                            , this->self->mirrorDimension () );
            if (intersection.mesh ().isEmpty ()) {
              this->self->state ().scene ().deleteMesh (intersection.mesh ());
            }
          }
          return ToolResponse::Redraw;
        }
        case Mode::DeleteSpheres: {
          SketchPathIntersection intersection;
          if (this->self->intersectsScene (e, intersection)) {
            this->self->snapshotSketchMeshes ();
            intersection.mesh ().deletePath ( intersection.path ()
                                            , this->self->mirrorDimension () );
            if (intersection.mesh ().isEmpty ()) {
              this->self->state ().scene ().deleteMesh (intersection.mesh ());
            }
          }
          return ToolResponse::Redraw;
        }
      }
    }
    return ToolResponse::None;
  }
};

DELEGATE_TOOL                         (ToolDeleteSketch)
DELEGATE_TOOL_RUN_MOUSE_RELEASE_EVENT (ToolDeleteSketch)
