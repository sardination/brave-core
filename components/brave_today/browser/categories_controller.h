#ifndef BRAVE_COMPONENTS_BRAVE_TODAY_BROWSER_CATEGORIES_CONTROLLER_H_
#define BRAVE_COMPONENTS_BRAVE_TODAY_BROWSER_CATEGORIES_CONTROLLER_H_

#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/containers/flat_map.h"
#include "brave/components/brave_today/browser/publishers_controller.h"
#include "brave/components/brave_today/common/brave_news.mojom-forward.h"
#include "components/prefs/pref_service.h"

namespace brave_news {
using Categories = base::flat_map<std::string, mojom::CategoryPtr>;
using CategoriesCallback = base::OnceCallback<void(Categories)>;

class CategoriesController {
 public:
  explicit CategoriesController(PrefService* prefs,
                                PublishersController* publishers_controller);
  ~CategoriesController();
  CategoriesController(const CategoriesController&) = delete;
  CategoriesController& operator=(const CategoriesController&) = delete;

  static Categories GetCategoriesFromPublishers(const Publishers& publishers,
                                                PrefService* prefs);
  void GetAllCategories(CategoriesCallback callback);
  mojom::CategoryPtr SetCategorySubscribed(const std::string& category_id,
                                           bool subscribed);
  bool GetCategorySubscribed(const std::string& category_id);

 private:
  raw_ptr<PrefService> prefs_;
  raw_ptr<PublishersController> publishers_controller_;
};
}  // namespace brave_news

#endif  // BRAVE_COMPONENTS_BRAVE_TODAY_BROWSER_CATEGORIES_CONTROLLER_H_
