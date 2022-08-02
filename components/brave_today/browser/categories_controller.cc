#include "brave/components/brave_today/browser/categories_controller.h"

#include <utility>

#include "base/bind.h"
#include "base/containers/contains.h"
#include "base/containers/flat_map.h"
#include "brave/components/brave_today/browser/brave_news_controller.h"
#include "brave/components/brave_today/browser/publishers_controller.h"
#include "brave/components/brave_today/common/brave_news.mojom-forward.h"
#include "brave/components/brave_today/common/brave_news.mojom.h"
#include "brave/components/brave_today/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"

namespace brave_news {
CategoriesController::CategoriesController(
    PrefService* prefs,
    PublishersController* publishers_controller)
    : prefs_(prefs), publishers_controller_(publishers_controller) {}

CategoriesController::~CategoriesController() = default;

Categories CategoriesController::GetCategoriesFromPublishers(
    const Publishers& publishers,
    PrefService* prefs) {
  Categories categories;
  auto* subscriptions = prefs->GetDictionary(prefs::kBraveNewsSubscriptions);

  for (const auto& it : publishers) {
    for (const auto& category_id : it.second->category_ids) {
      // Don't add categories which already exist.
      if (base::Contains(categories, category_id))
        continue;

      // TODO: When we have a separate category id & name use those.
      auto category = mojom::Category::New();
      category->category_id = category_id;
      category->category_name = category_id;
      category->subscribed =
          subscriptions->FindBoolKey(category_id).value_or(false);

      categories.insert({category_id, std::move(category)});
    }
  }
  return categories;
}

void CategoriesController::GetAllCategories(CategoriesCallback callback) {
  publishers_controller_->GetOrFetchPublishers(base::BindOnce(
      [](CategoriesCallback callback, PrefService* prefs,
         Publishers publishers) {
        auto result = GetCategoriesFromPublishers(publishers, prefs);
        std::move(callback).Run(std::move(std::move(result)));
      },
      std::move(callback), base::Unretained(prefs_)));
}

mojom::CategoryPtr CategoriesController::SetCategorySubscribed(const std::string& category_id,
                                                 bool subscribed) {
  DictionaryPrefUpdate update(prefs_, prefs::kBraveNewsSubscriptions);
  update->SetBoolKey(category_id, subscribed);

  // TODO: Maybe look this up somehow instead?
  auto result = mojom::Category::New();
  result->category_name = category_id;
  result->category_id = category_id;
  result->subscribed = subscribed;
  return result;
}

bool CategoriesController::GetCategorySubscribed(
    const std::string& category_id) {
  auto* subscriptions = prefs_->GetDictionary(prefs::kBraveNewsSubscriptions);
  return subscriptions->FindBoolKey(category_id).value_or(false);
}
}  // namespace brave_news
